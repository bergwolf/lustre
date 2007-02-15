 /* -*- mode: c; c-basic-offset: 8; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright (C) 2002, 2003 Cluster File Systems, Inc.
 * Author: Phil Schwan <phil@clusterfs.com>
 *         Peter Braam <braam@clusterfs.com>
 *         Mike Shaver <shaver@clusterfs.com>
 *
 *   This file is part of the Lustre file system, http://www.lustre.org
 *   Lustre is a trademark of Cluster File Systems, Inc.
 *
 *   You may have signed or agreed to another license before downloading
 *   this software.  If so, you are bound by the terms and conditions
 *   of that agreement, and the following does not apply to you.  See the
 *   LICENSE file included with this distribution for more information.
 *
 *   If you did not agree to a different license, then this copy of Lustre
 *   is open source software; you can redistribute it and/or modify it
 *   under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 *
 *   In either case, Lustre is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   license text for more details.
 */

#ifndef EXPORT_SYMTAB
# define EXPORT_SYMTAB
#endif
#define DEBUG_SUBSYSTEM S_LOV
#ifdef __KERNEL__
#include <libcfs/libcfs.h>
#else
#include <liblustre.h>
#endif

#include <obd_support.h>
#include <lustre_lib.h>
#include <lustre_net.h>
#include <lustre/lustre_idl.h>
#include <lustre_dlm.h>
#include <lustre_mds.h>
#include <obd_class.h>
#include <obd_lov.h>
#include <obd_ost.h>
#include <lprocfs_status.h>

#include "lov_internal.h"

/* Add log records for each OSC that this object is striped over, and return
 * cookies for each one.  We _would_ have nice abstraction here, except that
 * we need to keep cookies in stripe order, even if some are NULL, so that
 * the right cookies are passed back to the right OSTs at the client side.
 * Unset cookies should be all-zero (which will never occur naturally). */
static int lov_llog_origin_add(struct llog_ctxt *ctxt,
                        struct llog_rec_hdr *rec, struct lov_stripe_md *lsm,
                        struct llog_cookie *logcookies, int numcookies)
{
        struct obd_device *obd = ctxt->loc_obd;
        struct lov_obd *lov = &obd->u.lov;
        int i, rc = 0;
        ENTRY;

        LASSERTF(logcookies && numcookies >= lsm->lsm_stripe_count, 
                 "logcookies %p, numcookies %d lsm->lsm_stripe_count %d \n",
                 logcookies, numcookies, lsm->lsm_stripe_count);

        for (i = 0; i < lsm->lsm_stripe_count; i++) {
                struct lov_oinfo *loi = lsm->lsm_oinfo[i];
                struct obd_device *child = 
                        lov->lov_tgts[loi->loi_ost_idx]->ltd_exp->exp_obd; 
                struct llog_ctxt *cctxt = llog_get_context(child, ctxt->loc_idx);

                /* fill mds unlink/setattr log record */
                switch (rec->lrh_type) {
                case MDS_UNLINK_REC: {
                        struct llog_unlink_rec *lur = (struct llog_unlink_rec *)rec;
                        lur->lur_oid = loi->loi_id;
                        lur->lur_ogen = loi->loi_gr;
                        break;
                }
                case MDS_SETATTR_REC: {
                        struct llog_setattr_rec *lsr = (struct llog_setattr_rec *)rec;
                        lsr->lsr_oid = loi->loi_id;
                        lsr->lsr_ogen = loi->loi_gr;
                        break;
                }
                default:
                        break;
                }

                rc += llog_add(cctxt, rec, NULL, logcookies + rc,
                                numcookies - rc);
        }

        RETURN(rc);
}

static int lov_llog_origin_connect(struct llog_ctxt *ctxt, int count,
                                   struct llog_logid *logid,
                                   struct llog_gen *gen,
                                   struct obd_uuid *uuid)
{
        struct obd_device *obd = ctxt->loc_obd;
        struct lov_obd *lov = &obd->u.lov;
        int i, rc = 0, err = 0;
        ENTRY;

        lov_getref(obd);
        for (i = 0; i < count; i++) {
                struct obd_device *child;
                struct llog_ctxt *cctxt;
                
                if (!lov->lov_tgts[i] || !lov->lov_tgts[i]->ltd_active)
                        continue;
                if (uuid && !obd_uuid_equals(uuid, &lov->lov_tgts[i]->ltd_uuid))
                        continue;
                CDEBUG(D_CONFIG, "connect %d/%d\n", i, count);
                child = lov->lov_tgts[i]->ltd_exp->exp_obd;
                cctxt = llog_get_context(child, ctxt->loc_idx);
                rc = llog_connect(cctxt, 1, logid, gen, uuid);
                if (rc) {
                        CERROR("error osc_llog_connect tgt %d (%d)\n", i, rc);
                        if (!err) 
                                err = rc;
                }
        }
        lov_putref(obd);

        RETURN(err);
}

/* the replicators commit callback */
static int lov_llog_repl_cancel(struct llog_ctxt *ctxt, struct lov_stripe_md *lsm,
                          int count, struct llog_cookie *cookies, int flags)
{
        struct lov_obd *lov;
        struct obd_device *obd = ctxt->loc_obd;
        int rc = 0, i;
        ENTRY;

        LASSERT(lsm != NULL);
        LASSERT(count == lsm->lsm_stripe_count);

        lov = &obd->u.lov;
        lov_getref(obd);
        for (i = 0; i < count; i++, cookies++) {
                struct lov_oinfo *loi = lsm->lsm_oinfo[i];
                struct obd_device *child = 
                        lov->lov_tgts[loi->loi_ost_idx]->ltd_exp->exp_obd;
                struct llog_ctxt *cctxt = 
                        llog_get_context(child, ctxt->loc_idx);
                int err;

                err = llog_cancel(cctxt, NULL, 1, cookies, flags);
                if (err && lov->lov_tgts[loi->loi_ost_idx]->ltd_active) {
                        CERROR("error: objid "LPX64" subobj "LPX64
                               " on OST idx %d: rc = %d\n", lsm->lsm_object_id,
                               loi->loi_id, loi->loi_ost_idx, err);
                        if (!rc)
                                rc = err;
                }
        }
        lov_putref(obd);
        RETURN(rc);
}

static struct llog_operations lov_mds_ost_orig_logops = {
        lop_add: lov_llog_origin_add,
        lop_connect: lov_llog_origin_connect
};

static struct llog_operations lov_size_repl_logops = {
        lop_cancel: lov_llog_repl_cancel
};

int lov_llog_init(struct obd_device *obd, struct obd_device *tgt,
                  int count, struct llog_catid *logid, struct obd_uuid *uuid)
{
        struct lov_obd *lov = &obd->u.lov;
        struct obd_device *child;
        int i, rc = 0, err = 0;
        ENTRY;

        rc = llog_setup(obd, LLOG_MDS_OST_ORIG_CTXT, tgt, 0, NULL,
                        &lov_mds_ost_orig_logops);
        if (rc)
                RETURN(rc);

        rc = llog_setup(obd, LLOG_SIZE_REPL_CTXT, tgt, 0, NULL,
                        &lov_size_repl_logops);
        if (rc)
                RETURN(rc);

        lov_getref(obd);
        /* count may not match lov->desc.ld_tgt_count during dynamic ost add */
        for (i = 0; i < count; i++) {
                if (!lov->lov_tgts[i] || !lov->lov_tgts[i]->ltd_active)
                        continue;
                if (uuid && !obd_uuid_equals(uuid, &lov->lov_tgts[i]->ltd_uuid))
                        continue;
                CDEBUG(D_CONFIG, "init %d/%d\n", i, count);
                LASSERT(lov->lov_tgts[i]->ltd_exp);
                child = lov->lov_tgts[i]->ltd_exp->exp_obd;
                rc = obd_llog_init(child, tgt, 1, logid + i, uuid);
                if (rc) {
                        CERROR("error osc_llog_init idx %d osc '%s' tgt '%s' "
                               "(rc=%d)\n", i, child->obd_name, tgt->obd_name,
                               rc);
                        if (!err) 
                                err = rc;
                }
        }
        lov_putref(obd);
        RETURN(err);
}

int lov_llog_finish(struct obd_device *obd, int count)
{
        struct llog_ctxt *ctxt;
        int rc = 0, rc2 = 0;
        ENTRY;

        /* cleanup our llogs only if the ctxts have been setup
         * (client lov doesn't setup, mds lov does). */
        ctxt = llog_get_context(obd, LLOG_MDS_OST_ORIG_CTXT);
        if (ctxt)
                rc = llog_cleanup(ctxt);

        ctxt = llog_get_context(obd, LLOG_SIZE_REPL_CTXT);
        if (ctxt)
                rc2 = llog_cleanup(ctxt);
        if (!rc)
                rc = rc2;

        /* lov->tgt llogs are cleaned during osc_cleanup. */
        RETURN(rc);
}
