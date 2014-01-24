/*
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; If not, see
 * http://www.sun.com/software/products/lustre/docs/GPLv2.pdf
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *
 * GPL HEADER END
 */
/*
 * Copyright (c) 2007, 2010, Oracle and/or its affiliates. All rights reserved.
 * Use is subject to license terms.
 *
 * Copyright (c) 2011, 2013, Intel Corporation.
 */
/*
 * This file is part of Lustre, http://www.lustre.org/
 * Lustre is a trademark of Sun Microsystems, Inc.
 */

#ifndef __LINUX_FLD_H
#define __LINUX_FLD_H

/** \defgroup fld fld
 *
 * @{
 */

#include <lustre/lustre_idl.h>
#include <lustre_mdt.h>
#include <dt_object.h>

#include <libcfs/libcfs.h>

struct lu_client_fld;
struct lu_server_fld;
struct lu_fld_hash;
struct fld_cache;

extern const struct dt_index_features fld_index_features;
extern const char fld_index_name[];

/*
 * FLD (Fid Location Database) interface.
 */
enum {
        LUSTRE_CLI_FLD_HASH_DHT = 0,
        LUSTRE_CLI_FLD_HASH_RRB
};


struct lu_fld_target {
        cfs_list_t               ft_chain;
        struct obd_export       *ft_exp;
        struct lu_server_fld    *ft_srv;
        __u64                    ft_idx;
};

struct lu_server_fld {
        /**
         * Fld dir proc entry. */
        cfs_proc_dir_entry_t    *lsf_proc_dir;

        /**
         * /fld file object device */
        struct dt_object        *lsf_obj;

        /**
         * super sequence controller export, needed to forward fld
         * lookup  request. */
        struct obd_export       *lsf_control_exp;

        /**
         * Client FLD cache. */
        struct fld_cache        *lsf_cache;

        /**
         * Protect index modifications */
	struct mutex		lsf_lock;

        /**
         * Fld service name in form "fld-srv-lustre-MDTXXX" */
        char                     lsf_name[80];

};

struct lu_client_fld {
        /**
         * Client side proc entry. */
        cfs_proc_dir_entry_t    *lcf_proc_dir;

        /**
         * List of exports client FLD knows about. */
        cfs_list_t               lcf_targets;

        /**
         * Current hash to be used to chose an export. */
        struct lu_fld_hash      *lcf_hash;

        /**
         * Exports count. */
        int                      lcf_count;

        /**
         * Lock protecting exports list and fld_hash. */
	spinlock_t		 lcf_lock;

        /**
         * Client FLD cache. */
        struct fld_cache        *lcf_cache;

        /**
         * Client fld proc entry name. */
        char                     lcf_name[80];

        const struct lu_context *lcf_ctx;

        int                      lcf_flags;
};

/**
 * number of blocks to reserve for particular operations. Should be function of
 * ... something. Stub for now.
 */
enum {
        /* one insert operation can involve two delete and one insert */
        FLD_TXN_INDEX_INSERT_CREDITS  = 60,
        FLD_TXN_INDEX_DELETE_CREDITS  = 20,
};

int fld_query(struct com_thread_info *info);

/* Server methods */
int fld_server_init(const struct lu_env *env, struct lu_server_fld *fld,
		    struct dt_device *dt, const char *prefix, int mds_node_id,
		    int type);

void fld_server_fini(const struct lu_env *env, struct lu_server_fld *fld);

int fld_declare_server_create(const struct lu_env *env,
			      struct lu_server_fld *fld,
			      struct lu_seq_range *new,
			      struct thandle *th);

int fld_server_create(const struct lu_env *env,
		      struct lu_server_fld *fld,
		      struct lu_seq_range *add_range,
		      struct thandle *th);

int fld_insert_entry(const struct lu_env *env,
		     struct lu_server_fld *fld,
		     const struct lu_seq_range *range);

int fld_server_lookup(const struct lu_env *env, struct lu_server_fld *fld,
		      seqno_t seq, struct lu_seq_range *range);

/* Client methods */
int fld_client_init(struct lu_client_fld *fld,
                    const char *prefix, int hash);

void fld_client_fini(struct lu_client_fld *fld);

void fld_client_flush(struct lu_client_fld *fld);

int fld_client_lookup(struct lu_client_fld *fld, seqno_t seq, mdsno_t *mds,
                      __u32 flags, const struct lu_env *env);

int fld_client_create(struct lu_client_fld *fld,
                      struct lu_seq_range *range,
                      const struct lu_env *env);

int fld_client_delete(struct lu_client_fld *fld,
                      seqno_t seq,
                      const struct lu_env *env);

int fld_client_add_target(struct lu_client_fld *fld,
                          struct lu_fld_target *tar);

int fld_client_del_target(struct lu_client_fld *fld,
                          __u64 idx);

void fld_client_proc_fini(struct lu_client_fld *fld);

/** @} fld */

#endif
