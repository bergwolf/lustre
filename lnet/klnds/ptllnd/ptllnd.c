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
 * Copyright (c) 2012, Intel Corporation.
 */
/*
 * This file is part of Lustre, http://www.lustre.org/
 * Lustre is a trademark of Sun Microsystems, Inc.
 *
 * lnet/klnds/ptllnd/ptllnd.c
 *
 * Author: PJ Kirner <pjkirner@clusterfs.com>
 */

#include "ptllnd.h"

lnd_t kptllnd_lnd = {
        .lnd_type       = PTLLND,
        .lnd_startup    = kptllnd_startup,
        .lnd_shutdown   = kptllnd_shutdown,
        .lnd_ctl        = kptllnd_ctl,
        .lnd_query      = kptllnd_query,
        .lnd_send       = kptllnd_send,
        .lnd_recv       = kptllnd_recv,
        .lnd_eager_recv = kptllnd_eager_recv,
};

kptl_data_t kptllnd_data;

char *
kptllnd_ptlid2str(ptl_process_id_t id)
{
        static char    strs[64][32];
        static int     idx = 0;

        unsigned long  flags;
        char          *str;

	spin_lock_irqsave(&kptllnd_data.kptl_ptlid2str_lock, flags);
        str = strs[idx++];
        if (idx >= sizeof(strs)/sizeof(strs[0]))
                idx = 0;
	spin_unlock_irqrestore(&kptllnd_data.kptl_ptlid2str_lock, flags);

        snprintf(str, sizeof(strs[0]), FMT_PTLID, id.pid, id.nid);
        return str;
}

void 
kptllnd_assert_wire_constants (void)
{
        /* Wire protocol assertions generated by 'wirecheck'
         * running on Linux fedora 2.6.11-co-0.6.4 #1 Mon Jun 19 05:36:13 UTC 2006 i686 i686 i386 GNU
         * with gcc version 4.1.1 20060525 (Red Hat 4.1.1-1) */


        /* Constants... */
        CLASSERT (PTL_RESERVED_MATCHBITS == 0x100);
        CLASSERT (LNET_MSG_MATCHBITS == 0);
        CLASSERT (PTLLND_MSG_MAGIC == 0x50746C4E);
        CLASSERT (PTLLND_MSG_VERSION == 0x04);
        CLASSERT (PTLLND_RDMA_OK == 0x00);
        CLASSERT (PTLLND_RDMA_FAIL == 0x01);
        CLASSERT (PTLLND_MSG_TYPE_INVALID == 0x00);
        CLASSERT (PTLLND_MSG_TYPE_PUT == 0x01);
        CLASSERT (PTLLND_MSG_TYPE_GET == 0x02);
        CLASSERT (PTLLND_MSG_TYPE_IMMEDIATE == 0x03);
        CLASSERT (PTLLND_MSG_TYPE_NOOP == 0x04);
        CLASSERT (PTLLND_MSG_TYPE_HELLO == 0x05);
        CLASSERT (PTLLND_MSG_TYPE_NAK == 0x06);

        /* Checks for struct kptl_msg_t */
        CLASSERT ((int)sizeof(kptl_msg_t) == 136);
        CLASSERT ((int)offsetof(kptl_msg_t, ptlm_magic) == 0);
        CLASSERT ((int)sizeof(((kptl_msg_t *)0)->ptlm_magic) == 4);
        CLASSERT ((int)offsetof(kptl_msg_t, ptlm_version) == 4);
        CLASSERT ((int)sizeof(((kptl_msg_t *)0)->ptlm_version) == 2);
        CLASSERT ((int)offsetof(kptl_msg_t, ptlm_type) == 6);
        CLASSERT ((int)sizeof(((kptl_msg_t *)0)->ptlm_type) == 1);
        CLASSERT ((int)offsetof(kptl_msg_t, ptlm_credits) == 7);
        CLASSERT ((int)sizeof(((kptl_msg_t *)0)->ptlm_credits) == 1);
        CLASSERT ((int)offsetof(kptl_msg_t, ptlm_nob) == 8);
        CLASSERT ((int)sizeof(((kptl_msg_t *)0)->ptlm_nob) == 4);
        CLASSERT ((int)offsetof(kptl_msg_t, ptlm_cksum) == 12);
        CLASSERT ((int)sizeof(((kptl_msg_t *)0)->ptlm_cksum) == 4);
        CLASSERT ((int)offsetof(kptl_msg_t, ptlm_srcnid) == 16);
        CLASSERT ((int)sizeof(((kptl_msg_t *)0)->ptlm_srcnid) == 8);
        CLASSERT ((int)offsetof(kptl_msg_t, ptlm_srcstamp) == 24);
        CLASSERT ((int)sizeof(((kptl_msg_t *)0)->ptlm_srcstamp) == 8);
        CLASSERT ((int)offsetof(kptl_msg_t, ptlm_dstnid) == 32);
        CLASSERT ((int)sizeof(((kptl_msg_t *)0)->ptlm_dstnid) == 8);
        CLASSERT ((int)offsetof(kptl_msg_t, ptlm_dststamp) == 40);
        CLASSERT ((int)sizeof(((kptl_msg_t *)0)->ptlm_dststamp) == 8);
        CLASSERT ((int)offsetof(kptl_msg_t, ptlm_srcpid) == 48);
        CLASSERT ((int)sizeof(((kptl_msg_t *)0)->ptlm_srcpid) == 4);
        CLASSERT ((int)offsetof(kptl_msg_t, ptlm_dstpid) == 52);
        CLASSERT ((int)sizeof(((kptl_msg_t *)0)->ptlm_dstpid) == 4);
        CLASSERT ((int)offsetof(kptl_msg_t, ptlm_u.immediate) == 56);
        CLASSERT ((int)sizeof(((kptl_msg_t *)0)->ptlm_u.immediate) == 72);
        CLASSERT ((int)offsetof(kptl_msg_t, ptlm_u.rdma) == 56);
        CLASSERT ((int)sizeof(((kptl_msg_t *)0)->ptlm_u.rdma) == 80);
        CLASSERT ((int)offsetof(kptl_msg_t, ptlm_u.hello) == 56);
        CLASSERT ((int)sizeof(((kptl_msg_t *)0)->ptlm_u.hello) == 12);

        /* Checks for struct kptl_immediate_msg_t */
        CLASSERT ((int)sizeof(kptl_immediate_msg_t) == 72);
        CLASSERT ((int)offsetof(kptl_immediate_msg_t, kptlim_hdr) == 0);
        CLASSERT ((int)sizeof(((kptl_immediate_msg_t *)0)->kptlim_hdr) == 72);
        CLASSERT ((int)offsetof(kptl_immediate_msg_t, kptlim_payload[13]) == 85);
        CLASSERT ((int)sizeof(((kptl_immediate_msg_t *)0)->kptlim_payload[13]) == 1);

        /* Checks for struct kptl_rdma_msg_t */
        CLASSERT ((int)sizeof(kptl_rdma_msg_t) == 80);
        CLASSERT ((int)offsetof(kptl_rdma_msg_t, kptlrm_hdr) == 0);
        CLASSERT ((int)sizeof(((kptl_rdma_msg_t *)0)->kptlrm_hdr) == 72);
        CLASSERT ((int)offsetof(kptl_rdma_msg_t, kptlrm_matchbits) == 72);
        CLASSERT ((int)sizeof(((kptl_rdma_msg_t *)0)->kptlrm_matchbits) == 8);

        /* Checks for struct kptl_hello_msg_t */
        CLASSERT ((int)sizeof(kptl_hello_msg_t) == 12);
        CLASSERT ((int)offsetof(kptl_hello_msg_t, kptlhm_matchbits) == 0);
        CLASSERT ((int)sizeof(((kptl_hello_msg_t *)0)->kptlhm_matchbits) == 8);
        CLASSERT ((int)offsetof(kptl_hello_msg_t, kptlhm_max_msg_size) == 8);
        CLASSERT ((int)sizeof(((kptl_hello_msg_t *)0)->kptlhm_max_msg_size) == 4);
}

const char *kptllnd_evtype2str(int type)
{
#define DO_TYPE(x) case x: return #x;
        switch(type)
        {
                DO_TYPE(PTL_EVENT_GET_START);
                DO_TYPE(PTL_EVENT_GET_END);
                DO_TYPE(PTL_EVENT_PUT_START);
                DO_TYPE(PTL_EVENT_PUT_END);
                DO_TYPE(PTL_EVENT_REPLY_START);
                DO_TYPE(PTL_EVENT_REPLY_END);
                DO_TYPE(PTL_EVENT_ACK);
                DO_TYPE(PTL_EVENT_SEND_START);
                DO_TYPE(PTL_EVENT_SEND_END);
                DO_TYPE(PTL_EVENT_UNLINK);
        default:
                return "<unknown event type>";
        }
#undef DO_TYPE
}

const char *kptllnd_msgtype2str(int type)
{
#define DO_TYPE(x) case x: return #x;
        switch(type)
        {
                DO_TYPE(PTLLND_MSG_TYPE_INVALID);
                DO_TYPE(PTLLND_MSG_TYPE_PUT);
                DO_TYPE(PTLLND_MSG_TYPE_GET);
                DO_TYPE(PTLLND_MSG_TYPE_IMMEDIATE);
                DO_TYPE(PTLLND_MSG_TYPE_HELLO);
                DO_TYPE(PTLLND_MSG_TYPE_NOOP);
                DO_TYPE(PTLLND_MSG_TYPE_NAK);
        default:
                return "<unknown msg type>";
        }
#undef DO_TYPE
}

const char *kptllnd_errtype2str(int type)
{
#define DO_TYPE(x) case x: return #x;
        switch(type)
        {
                DO_TYPE(PTL_OK);
                DO_TYPE(PTL_SEGV);
                DO_TYPE(PTL_NO_SPACE);
                DO_TYPE(PTL_ME_IN_USE);
                DO_TYPE(PTL_NAL_FAILED);
                DO_TYPE(PTL_NO_INIT);
                DO_TYPE(PTL_IFACE_DUP);
                DO_TYPE(PTL_IFACE_INVALID);
                DO_TYPE(PTL_HANDLE_INVALID);
                DO_TYPE(PTL_MD_INVALID);
                DO_TYPE(PTL_ME_INVALID);
                DO_TYPE(PTL_PROCESS_INVALID);
                DO_TYPE(PTL_PT_INDEX_INVALID);
                DO_TYPE(PTL_SR_INDEX_INVALID);
                DO_TYPE(PTL_EQ_INVALID);
                DO_TYPE(PTL_EQ_DROPPED);
                DO_TYPE(PTL_EQ_EMPTY);
                DO_TYPE(PTL_MD_NO_UPDATE);
                DO_TYPE(PTL_FAIL);
                DO_TYPE(PTL_AC_INDEX_INVALID);
                DO_TYPE(PTL_MD_ILLEGAL);
                DO_TYPE(PTL_ME_LIST_TOO_LONG);
                DO_TYPE(PTL_MD_IN_USE);
                DO_TYPE(PTL_NI_INVALID);
                DO_TYPE(PTL_PID_INVALID);
                DO_TYPE(PTL_PT_FULL);
                DO_TYPE(PTL_VAL_FAILED);
                DO_TYPE(PTL_NOT_IMPLEMENTED);
                DO_TYPE(PTL_NO_ACK);
                DO_TYPE(PTL_EQ_IN_USE);
                DO_TYPE(PTL_PID_IN_USE);
                DO_TYPE(PTL_INV_EQ_SIZE);
                DO_TYPE(PTL_AGAIN);
        default:
                return "<unknown event type>";
        }
#undef DO_TYPE
}

__u32
kptllnd_cksum (void *ptr, int nob)
{
        char  *c  = ptr;
        __u32  sum = 0;

        while (nob-- > 0)
                sum = ((sum << 1) | (sum >> 31)) + *c++;

        /* ensure I don't return 0 (== no checksum) */
        return (sum == 0) ? 1 : sum;
}

void
kptllnd_init_msg(kptl_msg_t *msg, int type,
                 lnet_process_id_t target, int body_nob)
{
        msg->ptlm_type = type;
        msg->ptlm_nob  = (offsetof(kptl_msg_t, ptlm_u) + body_nob + 7) & ~7;
        msg->ptlm_dstpid = target.pid;
        msg->ptlm_dstnid = target.nid;
        msg->ptlm_srcpid = the_lnet.ln_pid;
        msg->ptlm_srcnid = kptllnd_ptl2lnetnid(target.nid,
                                               kptllnd_data.kptl_portals_id.nid);
        
        LASSERT(msg->ptlm_nob <= *kptllnd_tunables.kptl_max_msg_size);
}

void
kptllnd_msg_pack(kptl_msg_t *msg, kptl_peer_t *peer)
{
        msg->ptlm_magic    = PTLLND_MSG_MAGIC;
        msg->ptlm_version  = PTLLND_MSG_VERSION;
        /* msg->ptlm_type  Filled in kptllnd_init_msg()  */
        msg->ptlm_credits  = peer->peer_outstanding_credits;
        /* msg->ptlm_nob   Filled in kptllnd_init_msg()  */
        msg->ptlm_cksum    = 0;
        /* msg->ptlm_{src|dst}[pn]id Filled in kptllnd_init_msg */
        msg->ptlm_srcstamp = peer->peer_myincarnation;
        msg->ptlm_dststamp = peer->peer_incarnation;

        if (*kptllnd_tunables.kptl_checksum) {
                /* NB ptlm_cksum zero while computing cksum */
                msg->ptlm_cksum = kptllnd_cksum(msg, 
                                                offsetof(kptl_msg_t, ptlm_u));
        }
}

int
kptllnd_msg_unpack(kptl_msg_t *msg, int nob)
{
        const int hdr_size = offsetof(kptl_msg_t, ptlm_u);
        __u32     msg_cksum;
        __u16     msg_version;
        int       flip;

        /* 6 bytes are enough to have received magic + version */
        if (nob < 6) {
                CERROR("Very Short message: %d\n", nob);
                return -EPROTO;
        }

        /*
         * Determine if we need to flip
         */
        if (msg->ptlm_magic == PTLLND_MSG_MAGIC) {
                flip = 0;
        } else if (msg->ptlm_magic == __swab32(PTLLND_MSG_MAGIC)) {
                flip = 1;
        } else {
                CERROR("Bad magic: %08x\n", msg->ptlm_magic);
                return -EPROTO;
        }

        msg_version = flip ? __swab16(msg->ptlm_version) : msg->ptlm_version;

        if (msg_version != PTLLND_MSG_VERSION) {
                CERROR("Bad version: got %04x expected %04x\n",
                        (__u32)msg_version, PTLLND_MSG_VERSION);
                return -EPROTO;
        }

        if (nob < hdr_size) {
                CERROR("Short message: got %d, wanted at least %d\n",
                       nob, hdr_size);
                return -EPROTO;
        }

        /* checksum must be computed with
         * 1) ptlm_cksum zero and
         * 2) BEFORE anything gets modified/flipped
         */
        msg_cksum = flip ? __swab32(msg->ptlm_cksum) : msg->ptlm_cksum;
        msg->ptlm_cksum = 0;
        if (msg_cksum != 0 &&
            msg_cksum != kptllnd_cksum(msg, hdr_size)) {
                CERROR("Bad checksum\n");
                return -EPROTO;
        }

        msg->ptlm_version = msg_version;
        msg->ptlm_cksum = msg_cksum;
        
        if (flip) {
                /* These two are 1 byte long so we don't swap them
                   But check this assumtion*/
                CLASSERT (sizeof(msg->ptlm_type) == 1);
                CLASSERT (sizeof(msg->ptlm_credits) == 1);
                /* src & dst stamps are opaque cookies */
                __swab32s(&msg->ptlm_nob);
                __swab64s(&msg->ptlm_srcnid);
                __swab64s(&msg->ptlm_dstnid);
                __swab32s(&msg->ptlm_srcpid);
                __swab32s(&msg->ptlm_dstpid);
        }

        if (msg->ptlm_nob != nob) {
                CERROR("msg_nob corrupt: got 0x%08x, wanted %08x\n",
                       msg->ptlm_nob, nob);
                return -EPROTO;
        }

        switch(msg->ptlm_type)
        {
        case PTLLND_MSG_TYPE_PUT:
        case PTLLND_MSG_TYPE_GET:
                if (nob < hdr_size + sizeof(kptl_rdma_msg_t)) {
                        CERROR("Short rdma request: got %d, want %d\n",
                               nob, hdr_size + (int)sizeof(kptl_rdma_msg_t));
                        return -EPROTO;
                }

                if (flip)
                        __swab64s(&msg->ptlm_u.rdma.kptlrm_matchbits);

                if (msg->ptlm_u.rdma.kptlrm_matchbits < PTL_RESERVED_MATCHBITS) {
                        CERROR("Bad matchbits "LPX64"\n",
                               msg->ptlm_u.rdma.kptlrm_matchbits);
                        return -EPROTO;
                }
                break;

        case PTLLND_MSG_TYPE_IMMEDIATE:
                if (nob < offsetof(kptl_msg_t, 
                                   ptlm_u.immediate.kptlim_payload)) {
                        CERROR("Short immediate: got %d, want %d\n", nob,
                               (int)offsetof(kptl_msg_t, 
                                             ptlm_u.immediate.kptlim_payload));
                        return -EPROTO;
                }
                /* Do nothing */
                break;
                        
        case PTLLND_MSG_TYPE_NOOP:
        case PTLLND_MSG_TYPE_NAK:
                /* Do nothing */
                break;

        case PTLLND_MSG_TYPE_HELLO:
                if (nob < hdr_size + sizeof(kptl_hello_msg_t)) {
                        CERROR("Short hello: got %d want %d\n",
                               nob, hdr_size + (int)sizeof(kptl_hello_msg_t));
                        return -EPROTO;
                }
                if (flip) {
                        __swab64s(&msg->ptlm_u.hello.kptlhm_matchbits);
                        __swab32s(&msg->ptlm_u.hello.kptlhm_max_msg_size);
                }
                break;

        default:
                CERROR("Bad message type: 0x%02x\n", (__u32)msg->ptlm_type);
                return -EPROTO;
        }

        return 0;
}

int
kptllnd_ctl(lnet_ni_t *ni, unsigned int cmd, void *arg)
{
        kptl_net_t  *net = ni->ni_data;
        struct libcfs_ioctl_data *data = arg;
        int          rc = -EINVAL;

        CDEBUG(D_NET, ">>> kptllnd_ctl cmd=%u arg=%p\n", cmd, arg);

        /*
         * Validate that the context block is actually
         * pointing to this interface
         */
        LASSERT (ni == net->net_ni);

        switch(cmd) {
        case IOC_LIBCFS_DEL_PEER: {
                lnet_process_id_t id;
                
                id.nid = data->ioc_nid;
                id.pid = data->ioc_u32[1];
                
                rc = kptllnd_peer_del(id);
                break;
        }

        case IOC_LIBCFS_GET_PEER: {
                lnet_process_id_t   id = {.nid = LNET_NID_ANY,
                                          .pid = LNET_PID_ANY};
                __u64               incarnation = 0;
                __u64               next_matchbits = 0;
                __u64               last_matchbits_seen = 0;
                int                 state = 0;
                int                 sent_hello = 0;
                int                 refcount = 0;
                int                 nsendq = 0;
                int                 nactiveq = 0;
                int                 credits = 0;
                int                 outstanding_credits = 0;

                rc = kptllnd_get_peer_info(data->ioc_count, &id,
                                           &state, &sent_hello,
                                           &refcount, &incarnation,
                                           &next_matchbits, &last_matchbits_seen,
                                           &nsendq, &nactiveq,
                                           &credits, &outstanding_credits);
                /* wince... */
                data->ioc_nid = id.nid;
                data->ioc_net = state;
                data->ioc_flags  = sent_hello;
                data->ioc_count = refcount;
                data->ioc_u64[0] = incarnation;
                data->ioc_u32[0] = (__u32)next_matchbits;
                data->ioc_u32[1] = (__u32)(next_matchbits >> 32);
                data->ioc_u32[2] = (__u32)last_matchbits_seen;
                data->ioc_u32[3] = (__u32)(last_matchbits_seen >> 32);
                data->ioc_u32[4] = id.pid;
                data->ioc_u32[5] = (nsendq << 16) | nactiveq;
                data->ioc_u32[6] = (credits << 16) | outstanding_credits;
                break;
        }
                
        default:
                rc=-EINVAL;
                break;
        }
        CDEBUG(D_NET, "<<< kptllnd_ctl rc=%d\n", rc);
        return rc;
}

void
kptllnd_query (lnet_ni_t *ni, lnet_nid_t nid, cfs_time_t *when)
{
        kptl_net_t        *net = ni->ni_data;
        kptl_peer_t       *peer = NULL;
        lnet_process_id_t  id = {.nid = nid, .pid = LUSTRE_SRV_LNET_PID};
        unsigned long      flags;

        /* NB: kptllnd_find_target connects to peer if necessary */
        if (kptllnd_find_target(net, id, &peer) != 0)
                return;

	spin_lock_irqsave(&peer->peer_lock, flags);
        if (peer->peer_last_alive != 0)
                *when = peer->peer_last_alive;
	spin_unlock_irqrestore(&peer->peer_lock, flags);
        kptllnd_peer_decref(peer);
        return;
}

void
kptllnd_base_shutdown (void)
{
        int               i;
        ptl_err_t         prc;
        unsigned long     flags;
        lnet_process_id_t process_id;

	read_lock(&kptllnd_data.kptl_net_rw_lock);
        LASSERT (list_empty(&kptllnd_data.kptl_nets));
	read_unlock(&kptllnd_data.kptl_net_rw_lock);

        switch (kptllnd_data.kptl_init) {
        default:
                LBUG();

        case PTLLND_INIT_ALL:
        case PTLLND_INIT_DATA:
                /* stop receiving */
                kptllnd_rx_buffer_pool_fini(&kptllnd_data.kptl_rx_buffer_pool);
                LASSERT (list_empty(&kptllnd_data.kptl_sched_rxq));
                LASSERT (list_empty(&kptllnd_data.kptl_sched_rxbq));

                /* lock to interleave cleanly with peer birth/death */
		write_lock_irqsave(&kptllnd_data.kptl_peer_rw_lock, flags);
                LASSERT (kptllnd_data.kptl_shutdown == 0);
                kptllnd_data.kptl_shutdown = 1; /* phase 1 == destroy peers */
                /* no new peers possible now */
		write_unlock_irqrestore(&kptllnd_data.kptl_peer_rw_lock,
                                            flags);

                /* nuke all existing peers */
                process_id.nid = LNET_NID_ANY;
                process_id.pid = LNET_PID_ANY;
                kptllnd_peer_del(process_id);

		read_lock_irqsave(&kptllnd_data.kptl_peer_rw_lock, flags);

                LASSERT (kptllnd_data.kptl_n_active_peers == 0);

                i = 2;
                while (kptllnd_data.kptl_npeers != 0) {
                        i++;
                        CDEBUG(((i & (-i)) == i) ? D_WARNING : D_NET,
                               "Waiting for %d peers to terminate\n",
                               kptllnd_data.kptl_npeers);

			read_unlock_irqrestore(&kptllnd_data.kptl_peer_rw_lock,
                                                   flags);

                        cfs_pause(cfs_time_seconds(1));

			read_lock_irqsave(&kptllnd_data.kptl_peer_rw_lock,
                                              flags);
                }

                LASSERT (list_empty(&kptllnd_data.kptl_closing_peers));
                LASSERT (list_empty(&kptllnd_data.kptl_zombie_peers));
                LASSERT (kptllnd_data.kptl_peers != NULL);
                for (i = 0; i < kptllnd_data.kptl_peer_hash_size; i++)
                        LASSERT (list_empty (&kptllnd_data.kptl_peers[i]));

		read_unlock_irqrestore(&kptllnd_data.kptl_peer_rw_lock,
                                           flags);
                CDEBUG(D_NET, "All peers deleted\n");

                /* Shutdown phase 2: kill the daemons... */
                kptllnd_data.kptl_shutdown = 2;
                cfs_mb();

                i = 2;
                while (cfs_atomic_read (&kptllnd_data.kptl_nthreads) != 0) {
                        /* Wake up all threads*/
                        cfs_waitq_broadcast(&kptllnd_data.kptl_sched_waitq);
                        cfs_waitq_broadcast(&kptllnd_data.kptl_watchdog_waitq);

                        i++;
                        CDEBUG(((i & (-i)) == i) ? D_WARNING : D_NET, /* power of 2? */
                               "Waiting for %d threads to terminate\n",
                               cfs_atomic_read(&kptllnd_data.kptl_nthreads));
                        cfs_pause(cfs_time_seconds(1));
                }

                CDEBUG(D_NET, "All Threads stopped\n");
                LASSERT(list_empty(&kptllnd_data.kptl_sched_txq));

                kptllnd_cleanup_tx_descs();

                /* Nothing here now, but libcfs might soon require
                 * us to explicitly destroy wait queues and semaphores
                 * that would be done here */

                /* fall through */

        case PTLLND_INIT_NOTHING:
                CDEBUG(D_NET, "PTLLND_INIT_NOTHING\n");
                break;
        }

        if (!PtlHandleIsEqual(kptllnd_data.kptl_eqh, PTL_INVALID_HANDLE)) {
                prc = PtlEQFree(kptllnd_data.kptl_eqh);
                if (prc != PTL_OK)
                        CERROR("Error %s(%d) freeing portals EQ\n",
                               kptllnd_errtype2str(prc), prc);
        }

        if (!PtlHandleIsEqual(kptllnd_data.kptl_nih, PTL_INVALID_HANDLE)) {
                prc = PtlNIFini(kptllnd_data.kptl_nih);
                if (prc != PTL_OK)
                        CERROR("Error %s(%d) finalizing portals NI\n",
                               kptllnd_errtype2str(prc), prc);
        }

        LASSERT (cfs_atomic_read(&kptllnd_data.kptl_ntx) == 0);
        LASSERT (list_empty(&kptllnd_data.kptl_idle_txs));

        if (kptllnd_data.kptl_rx_cache != NULL)
                cfs_mem_cache_destroy(kptllnd_data.kptl_rx_cache);

        if (kptllnd_data.kptl_peers != NULL)
                LIBCFS_FREE(kptllnd_data.kptl_peers,
                            sizeof (struct list_head) *
                            kptllnd_data.kptl_peer_hash_size);

        if (kptllnd_data.kptl_nak_msg != NULL)
                LIBCFS_FREE(kptllnd_data.kptl_nak_msg,
                            offsetof(kptl_msg_t, ptlm_u));

        memset(&kptllnd_data, 0, sizeof(kptllnd_data));
        PORTAL_MODULE_UNUSE;
        return;
}

int
kptllnd_base_startup (void)
{
        int             i;
        int                rc;
        int             spares;
        struct timeval  tv;
        lnet_process_id_t  target;
        ptl_err_t       ptl_rc;
	char            name[16];

        if (*kptllnd_tunables.kptl_max_procs_per_node < 1) {
                CERROR("max_procs_per_node must be >= 1\n");
                return -EINVAL;
        }

        if (*kptllnd_tunables.kptl_peertxcredits > PTLLND_MSG_MAX_CREDITS) {
                CERROR("peercredits must be <= %d\n", PTLLND_MSG_MAX_CREDITS);
                return -EINVAL;
        }

        *kptllnd_tunables.kptl_max_msg_size &= ~7;
        if (*kptllnd_tunables.kptl_max_msg_size < PTLLND_MIN_BUFFER_SIZE)
                *kptllnd_tunables.kptl_max_msg_size = PTLLND_MIN_BUFFER_SIZE;

        CLASSERT ((PTLLND_MIN_BUFFER_SIZE & 7) == 0);
        CLASSERT (sizeof(kptl_msg_t) <= PTLLND_MIN_BUFFER_SIZE);

        /* Zero pointers, flags etc; put everything into a known state. */
        memset (&kptllnd_data, 0, sizeof (kptllnd_data));

        LIBCFS_ALLOC(kptllnd_data.kptl_nak_msg, offsetof(kptl_msg_t, ptlm_u));
        if (kptllnd_data.kptl_nak_msg == NULL) {
                CERROR("Can't allocate NAK msg\n");
                return -ENOMEM;
        }
        memset(kptllnd_data.kptl_nak_msg, 0, offsetof(kptl_msg_t, ptlm_u));

        kptllnd_data.kptl_eqh = PTL_INVALID_HANDLE;
        kptllnd_data.kptl_nih = PTL_INVALID_HANDLE;

	rwlock_init(&kptllnd_data.kptl_net_rw_lock);
        INIT_LIST_HEAD(&kptllnd_data.kptl_nets);

        /* Setup the sched locks/lists/waitq */
	spin_lock_init(&kptllnd_data.kptl_sched_lock);
        cfs_waitq_init(&kptllnd_data.kptl_sched_waitq);
        INIT_LIST_HEAD(&kptllnd_data.kptl_sched_txq);
        INIT_LIST_HEAD(&kptllnd_data.kptl_sched_rxq);
        INIT_LIST_HEAD(&kptllnd_data.kptl_sched_rxbq);

        /* Init kptl_ptlid2str_lock before any call to kptllnd_ptlid2str */
	spin_lock_init(&kptllnd_data.kptl_ptlid2str_lock);

        /* Setup the tx locks/lists */
	spin_lock_init(&kptllnd_data.kptl_tx_lock);
        INIT_LIST_HEAD(&kptllnd_data.kptl_idle_txs);
        cfs_atomic_set(&kptllnd_data.kptl_ntx, 0);

        /* Uptick the module reference count */
        PORTAL_MODULE_USE;

        kptllnd_data.kptl_expected_peers =
                *kptllnd_tunables.kptl_max_nodes *
                *kptllnd_tunables.kptl_max_procs_per_node;

        /*
         * Initialize the Network interface instance
         * We use the default because we don't have any
         * way to choose a better interface.
         * Requested and actual limits are ignored.
         */
        ptl_rc = PtlNIInit(
#ifdef _USING_LUSTRE_PORTALS_
                PTL_IFACE_DEFAULT,
#else
                CRAY_KERN_NAL,
#endif
                *kptllnd_tunables.kptl_pid, NULL, NULL,
                &kptllnd_data.kptl_nih);

        /*
         * Note: PTL_IFACE_DUP simply means that the requested
         * interface was already inited and that we're sharing it.
         * Which is ok.
         */
        if (ptl_rc != PTL_OK && ptl_rc != PTL_IFACE_DUP) {
                CERROR ("PtlNIInit: error %s(%d)\n",
                        kptllnd_errtype2str(ptl_rc), ptl_rc);
                rc = -EINVAL;
                goto failed;
        }

        /* NB eq size irrelevant if using a callback */
        ptl_rc = PtlEQAlloc(kptllnd_data.kptl_nih,
                            8,                       /* size */
                            kptllnd_eq_callback,     /* handler callback */
                            &kptllnd_data.kptl_eqh); /* output handle */
        if (ptl_rc != PTL_OK) {
                CERROR("PtlEQAlloc failed %s(%d)\n",
                       kptllnd_errtype2str(ptl_rc), ptl_rc);
                rc = -ENOMEM;
                goto failed;
        }

        /* Fetch the lower NID */
        ptl_rc = PtlGetId(kptllnd_data.kptl_nih,
                          &kptllnd_data.kptl_portals_id);
        if (ptl_rc != PTL_OK) {
                CERROR ("PtlGetID: error %s(%d)\n",
                        kptllnd_errtype2str(ptl_rc), ptl_rc);
                rc = -EINVAL;
                goto failed;
        }

        if (kptllnd_data.kptl_portals_id.pid != *kptllnd_tunables.kptl_pid) {
                /* The kernel ptllnd must have the expected PID */
                CERROR("Unexpected PID: %u (%u expected)\n",
                       kptllnd_data.kptl_portals_id.pid,
                       *kptllnd_tunables.kptl_pid);
                rc = -EINVAL;
                goto failed;
        }

        /* Initialized the incarnation - it must be for-all-time unique, even
         * accounting for the fact that we increment it when we disconnect a
         * peer that's using it */
        cfs_gettimeofday(&tv);
        kptllnd_data.kptl_incarnation = (((__u64)tv.tv_sec) * 1000000) +
                                        tv.tv_usec;
        CDEBUG(D_NET, "Incarnation="LPX64"\n", kptllnd_data.kptl_incarnation);

        target.nid = LNET_NID_ANY;
        target.pid = LNET_PID_ANY; /* NB target for NAK doesn't matter */
        kptllnd_init_msg(kptllnd_data.kptl_nak_msg, PTLLND_MSG_TYPE_NAK, target, 0);
        kptllnd_data.kptl_nak_msg->ptlm_magic    = PTLLND_MSG_MAGIC;
        kptllnd_data.kptl_nak_msg->ptlm_version  = PTLLND_MSG_VERSION;
        kptllnd_data.kptl_nak_msg->ptlm_srcpid   = the_lnet.ln_pid;
        kptllnd_data.kptl_nak_msg->ptlm_srcstamp = kptllnd_data.kptl_incarnation;

	rwlock_init(&kptllnd_data.kptl_peer_rw_lock);
        cfs_waitq_init(&kptllnd_data.kptl_watchdog_waitq);
        INIT_LIST_HEAD(&kptllnd_data.kptl_closing_peers);
        INIT_LIST_HEAD(&kptllnd_data.kptl_zombie_peers);

        /* Allocate and setup the peer hash table */
        kptllnd_data.kptl_peer_hash_size =
                *kptllnd_tunables.kptl_peer_hash_table_size;
        LIBCFS_ALLOC(kptllnd_data.kptl_peers,
                     sizeof(struct list_head) *
                     kptllnd_data.kptl_peer_hash_size);
        if (kptllnd_data.kptl_peers == NULL) {
                CERROR("Failed to allocate space for peer hash table size=%d\n",
                        kptllnd_data.kptl_peer_hash_size);
                rc = -ENOMEM;
                goto failed;
        }
        for (i = 0; i < kptllnd_data.kptl_peer_hash_size; i++)
                INIT_LIST_HEAD(&kptllnd_data.kptl_peers[i]);

        kptllnd_rx_buffer_pool_init(&kptllnd_data.kptl_rx_buffer_pool);

        kptllnd_data.kptl_rx_cache =
                cfs_mem_cache_create("ptllnd_rx",
                                     sizeof(kptl_rx_t) + 
                                     *kptllnd_tunables.kptl_max_msg_size,
                                     0,    /* offset */
                                     0);   /* flags */
        if (kptllnd_data.kptl_rx_cache == NULL) {
                CERROR("Can't create slab for RX descriptors\n");
                rc = -ENOMEM;
                goto failed;
        }

        /* lists/ptrs/locks initialised */
        kptllnd_data.kptl_init = PTLLND_INIT_DATA;

        /*****************************************************/

        rc = kptllnd_setup_tx_descs();
        if (rc != 0) {
                CERROR("Can't pre-allocate %d TX descriptors: %d\n",
                       *kptllnd_tunables.kptl_ntx, rc);
                goto failed;
        }
        
        /* Start the scheduler threads for handling incoming requests.  No need
         * to advance the state because this will be automatically cleaned up
         * now that PTLLND_INIT_DATA state has been entered */
        CDEBUG(D_NET, "starting %d scheduler threads\n", PTLLND_N_SCHED);
        for (i = 0; i < PTLLND_N_SCHED; i++) {
		snprintf(name, sizeof(name), "kptllnd_sd_%02d", i);
                rc = kptllnd_thread_start(kptllnd_scheduler, (void *)((long)i));
                if (rc != 0) {
                        CERROR("Can't spawn scheduler[%d]: %d\n", i, rc);
                        goto failed;
                }
        }

	snprintf(name, sizeof(name), "kptllnd_wd_%02d", i);
	rc = kptllnd_thread_start(kptllnd_watchdog, NULL, name);
        if (rc != 0) {
                CERROR("Can't spawn watchdog: %d\n", rc);
                goto failed;
        }

        /* Ensure that 'rxb_nspare' buffers can be off the net (being emptied)
         * and we will still have enough buffers posted for all our peers */
        spares = *kptllnd_tunables.kptl_rxb_nspare *
                 ((*kptllnd_tunables.kptl_rxb_npages * PAGE_SIZE)/
                  *kptllnd_tunables.kptl_max_msg_size);

        /* reserve and post the buffers */
        rc = kptllnd_rx_buffer_pool_reserve(&kptllnd_data.kptl_rx_buffer_pool,
                                            kptllnd_data.kptl_expected_peers +
                                            spares);
        if (rc != 0) {
                CERROR("Can't reserve RX Buffer pool: %d\n", rc);
                goto failed;
        }

        /* flag everything initialised */
        kptllnd_data.kptl_init = PTLLND_INIT_ALL;

        /*****************************************************/

        if (*kptllnd_tunables.kptl_checksum)
                CWARN("Checksumming enabled\n");

        CDEBUG(D_NET, "<<< kptllnd_base_startup SUCCESS\n");
        return 0;

 failed:
        CERROR("kptllnd_base_startup failed: %d\n", rc);
        kptllnd_base_shutdown();
        return rc;
}

int
kptllnd_startup (lnet_ni_t *ni)
{
        int         rc;
        kptl_net_t *net;

        LASSERT (ni->ni_lnd == &kptllnd_lnd);

        if (kptllnd_data.kptl_init == PTLLND_INIT_NOTHING) {
                rc = kptllnd_base_startup();
                if (rc != 0)
                        return rc;
        }

        LIBCFS_ALLOC(net, sizeof(*net));
        ni->ni_data = net;
        if (net == NULL) {
                CERROR("Can't allocate kptl_net_t\n");
                rc = -ENOMEM;
                goto failed;
        }
        memset(net, 0, sizeof(*net));
        net->net_ni = ni;

        ni->ni_maxtxcredits   = *kptllnd_tunables.kptl_credits;
        ni->ni_peertxcredits  = *kptllnd_tunables.kptl_peertxcredits;
        ni->ni_peerrtrcredits = *kptllnd_tunables.kptl_peerrtrcredits;
        ni->ni_nid = kptllnd_ptl2lnetnid(ni->ni_nid,
                                         kptllnd_data.kptl_portals_id.nid);
        CDEBUG(D_NET, "ptl id=%s, lnet id=%s\n",
               kptllnd_ptlid2str(kptllnd_data.kptl_portals_id),
               libcfs_nid2str(ni->ni_nid));

        /* NB LNET_NIDNET(ptlm_srcnid) of NAK doesn't matter in case of
         * multiple NIs */
        kptllnd_data.kptl_nak_msg->ptlm_srcnid = ni->ni_nid;

        cfs_atomic_set(&net->net_refcount, 1);
	write_lock(&kptllnd_data.kptl_net_rw_lock);
        list_add_tail(&net->net_list, &kptllnd_data.kptl_nets);
	write_unlock(&kptllnd_data.kptl_net_rw_lock);
        return 0;

 failed:
        kptllnd_shutdown(ni);
        return rc;
}

void
kptllnd_shutdown (lnet_ni_t *ni)
{
        kptl_net_t    *net = ni->ni_data;
        int               i;
        unsigned long     flags;

        LASSERT (kptllnd_data.kptl_init == PTLLND_INIT_ALL);

        CDEBUG(D_MALLOC, "before LND cleanup: kmem %d\n",
               cfs_atomic_read (&libcfs_kmemory));

        if (net == NULL)
                goto out;

        LASSERT (ni == net->net_ni);
        LASSERT (!net->net_shutdown);
        LASSERT (!list_empty(&net->net_list));
        LASSERT (cfs_atomic_read(&net->net_refcount) != 0);
        ni->ni_data = NULL;
        net->net_ni = NULL;

	write_lock(&kptllnd_data.kptl_net_rw_lock);
        kptllnd_net_decref(net);
        list_del_init(&net->net_list);
	write_unlock(&kptllnd_data.kptl_net_rw_lock);

        /* Can't nuke peers here - they are shared among all NIs */
	write_lock_irqsave(&kptllnd_data.kptl_peer_rw_lock, flags);
        net->net_shutdown = 1;   /* Order with peer creation */
	write_unlock_irqrestore(&kptllnd_data.kptl_peer_rw_lock, flags);

        i = 2;
        while (cfs_atomic_read(&net->net_refcount) != 0) {
                        i++;
                        CDEBUG(((i & (-i)) == i) ? D_WARNING : D_NET,
                       "Waiting for %d references to drop\n",
                       cfs_atomic_read(&net->net_refcount));

                       cfs_pause(cfs_time_seconds(1));
                }

        LIBCFS_FREE(net, sizeof(*net));
out:
        /* NB no locking since I don't race with writers */
        if (list_empty(&kptllnd_data.kptl_nets))
                kptllnd_base_shutdown();
        CDEBUG(D_MALLOC, "after LND cleanup: kmem %d\n",
               cfs_atomic_read (&libcfs_kmemory));
        return;
}

int __init
kptllnd_module_init (void)
{
        int    rc;

        kptllnd_assert_wire_constants();

        rc = kptllnd_tunables_init();
        if (rc != 0)
                return rc;

        kptllnd_init_ptltrace();

        lnet_register_lnd(&kptllnd_lnd);

        return 0;
}

void __exit
kptllnd_module_fini (void)
{
        lnet_unregister_lnd(&kptllnd_lnd);
        kptllnd_tunables_fini();
}

MODULE_AUTHOR("Sun Microsystems, Inc. <http://www.lustre.org/>");
MODULE_DESCRIPTION("Kernel Portals LND v1.00");
MODULE_LICENSE("GPL");

module_init(kptllnd_module_init);
module_exit(kptllnd_module_fini);
