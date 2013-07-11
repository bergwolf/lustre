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
 * Copyright (c) 2008, 2010, Oracle and/or its affiliates. All rights reserved.
 * Use is subject to license terms.
 *
 * Copyright (c) 2012, 2013, Intel Corporation.
 */
/*
 * This file is part of Lustre, http://www.lustre.org/
 * Lustre is a trademark of Sun Microsystems, Inc.
 */

#ifndef __LIBCFS_LINUX_PORTALS_COMPAT_H__
#define __LIBCFS_LINUX_PORTALS_COMPAT_H__

#include <net/sock.h>

#define SIGNAL_MASK_LOCK(task, flags)                                  \
	spin_lock_irqsave(&task->sighand->siglock, flags)
#define SIGNAL_MASK_UNLOCK(task, flags)                                \
	spin_unlock_irqrestore(&task->sighand->siglock, flags)
#define USERMODEHELPER(path, argv, envp)                               \
	call_usermodehelper(path, argv, envp, 1)

#ifndef __user
#define __user
#endif

#ifndef __fls
#define __cfs_fls fls
#else
#define __cfs_fls __fls
#endif

#ifdef HAVE_5ARGS_SYSCTL_PROC_HANDLER
#define ll_proc_dointvec(table, write, filp, buffer, lenp, ppos)        \
        proc_dointvec(table, write, buffer, lenp, ppos);

#define ll_proc_dolongvec(table, write, filp, buffer, lenp, ppos)        \
        proc_doulongvec_minmax(table, write, buffer, lenp, ppos);
#define ll_proc_dostring(table, write, filp, buffer, lenp, ppos)        \
        proc_dostring(table, write, buffer, lenp, ppos);
#define LL_PROC_PROTO(name)						\
	name(struct ctl_table *table, int write,			\
	     void __user *buffer, size_t *lenp, loff_t *ppos)
#else
#define ll_proc_dointvec(table, write, filp, buffer, lenp, ppos)        \
        proc_dointvec(table, write, filp, buffer, lenp, ppos);

#define ll_proc_dolongvec(table, write, filp, buffer, lenp, ppos)        \
        proc_doulongvec_minmax(table, write, filp, buffer, lenp, ppos);
#define ll_proc_dostring(table, write, filp, buffer, lenp, ppos)        \
        proc_dostring(table, write, filp, buffer, lenp, ppos);
#define LL_PROC_PROTO(name)						\
	name(struct ctl_table *table, int write, struct file *filp,	\
	     void __user *buffer, size_t *lenp, loff_t *ppos)
#endif
#define DECLARE_LL_PROC_PPOS_DECL

/* helper for sysctl handlers */
int proc_call_handler(void *data, int write,
                      loff_t *ppos, void *buffer, size_t *lenp,
                      int (*handler)(void *data, int write,
                                     loff_t pos, void *buffer, int len));

#ifndef HAVE_SK_SLEEP
static inline wait_queue_head_t *sk_sleep(struct sock *sk)
{
	return sk->sk_sleep;
}
#endif

#ifdef HAVE_INIT_NET
# define DEFAULT_NET	(&init_net)
#else
/* some broken backports */
# define DEFAULT_NET	(NULL)
#endif

#ifndef HAVE_STRUCT_CRED
# define current_cred() (current)
# define current_cred_xxx(xxx)                    \
({                                                \
	current->xxx;                             \
})
# ifndef HAVE_CRED_WRAPPERS
#  define current_uid()           (current_cred_xxx(uid))
#  define current_gid()           (current_cred_xxx(gid))
#  define current_euid()          (current_cred_xxx(euid))
#  define current_egid()          (current_cred_xxx(egid))
#  define current_suid()          (current_cred_xxx(suid))
#  define current_sgid()          (current_cred_xxx(sgid))
#  define current_fsuid()         (current_cred_xxx(fsuid))
#  define current_fsgid()         (current_cred_xxx(fsgid))
#  define current_cap()           (current_cred_xxx(cap_effective))
# endif /* HAVE_CRED_WRAPPERS */
# define current_user()          (current_cred_xxx(user))
# define current_user_ns()       (current_cred_xxx(user)->user_ns)
# define current_security()      (current_cred_xxx(security))
# define cred task_struct
# define prepare_creds() (current)
# define commit_creds(a)
#endif /* HAVE_STRUCT_CRED */

#endif /* _PORTALS_COMPAT_H */
