/*
 * Copyright (c) 2013, EMC Corporation.
 *
 * This file is part of Lustre, http://www.lustre.org/
 *
 * Implementation of fscache hooks for llite.
 *
 *   Author: Peng Tao <bergwolf@gmail.com>
 */
#include <lustre_lite.h>
#include "vvp_internal.h"
#include "fscache.h"

struct fscache_netfs ll_fscache_netfs = {
	.name		= "lustre",
	.version	= 0,
};

int ll_fscache_register(void)
{
	return fscache_register_netfs(&ll_fscache_netfs);
}

void ll_fscache_unregister(void)
{
	fscache_unregister_netfs(&ll_fscache_netfs);
}

static uint16_t ll_fscache_super_get_key(const void *cookie_netfs_data,
					 void *buffer, uint16_t bufmax)
{
	const struct ll_sb_info		*sbi = cookie_netfs_data;
	if (bufmax < sizeof(sbi->ll_sb_uuid))
		return 0;

	memcpy(buffer, &sbi->ll_sb_uuid, sizeof(sbi->ll_sb_uuid));
	return sizeof(sbi->ll_sb_uuid);
}

const struct fscache_cookie_def ll_fscache_super_index_def = {
	.name		= "LUSTRE.super",
	.type		= FSCACHE_COOKIE_TYPE_INDEX,
	.get_key	= ll_fscache_super_get_key,
};

static uint16_t ll_fscache_inode_get_key(const void *cookie_netfs_data,
					 void *buffer, uint16_t bufmax)
{
	const struct ll_inode_info	*lli = cookie_netfs_data;

	if (bufmax < sizeof(lli->lli_fid))
		return 0;
	memcpy(buffer, &lli->lli_fid, sizeof(lli->lli_fid));
	return sizeof(lli->lli_fid);
}

static void ll_fscache_inode_get_attr(const void *cookie_netfs_data,
				      uint64_t *size)
{
	const struct ll_inode_info	*lli = cookie_netfs_data;

	*size = i_size_read(&lli->lli_vfs_inode);
}

struct ll_fscache_inode_auxdata {
	struct timespec last_write_time;
	struct timespec last_change_time;
	loff_t		size;
};

static uint16_t
ll_fscache_inode_get_aux(const void *cookie_netfs_data,
			 void *buffer, uint16_t bufmax)
{
	const struct ll_inode_info	*lli = cookie_netfs_data;
	const struct inode		*inode = &lli->lli_vfs_inode;
	struct ll_fscache_inode_auxdata	auxdata;

	if (bufmax < sizeof(auxdata))
		return 0;

	auxdata.last_write_time = inode->i_mtime;
	auxdata.last_change_time = inode->i_ctime;
	auxdata.size = (lli->lli_flags & LLIF_MDS_SIZE_LOCK) ?
						i_size_read(inode) : 0;
	memcpy(buffer, &auxdata, sizeof(auxdata));

	return sizeof(auxdata);
}

static enum
fscache_checkaux ll_fscache_inode_check_aux(void *cookie_netfs_data,
					    const void *data,
					    uint16_t datalen)
{
	const struct ll_inode_info	*lli = cookie_netfs_data;
	const struct inode		*inode = &lli->lli_vfs_inode;
	struct ll_fscache_inode_auxdata	aux;
	enum fscache_checkaux	ret;

	if (datalen != sizeof(aux))
		return FSCACHE_CHECKAUX_OBSOLETE;

	aux.last_write_time = inode->i_mtime;
	aux.last_change_time = inode->i_ctime;
	aux.size = (lli->lli_flags & LLIF_MDS_SIZE_LOCK) ?
						i_size_read(inode) : 0;

	if (memcmp(data, &aux, datalen) == 0)
		ret = FSCACHE_CHECKAUX_OKAY;
	else
		ret = FSCACHE_CHECKAUX_OBSOLETE;
	return ret;
}

static void ll_fscache_inode_now_uncached(void *cookie_netfs_data)
{
	const struct ll_inode_info	*lli = cookie_netfs_data;
        struct pagevec			pvec;
        pgoff_t				first;
        int				loop, nr_pages;

        pagevec_init(&pvec, 0);
        first = 0;

        for (;;) {
                nr_pages = pagevec_lookup(&pvec,
                                          lli->lli_vfs_inode.i_mapping, first,
                                          PAGEVEC_SIZE - pagevec_count(&pvec));
                if (!nr_pages)
                        break;

                for (loop = 0; loop < nr_pages; loop++)
                        ClearPageFsCache(pvec.pages[loop]);

                first = pvec.pages[nr_pages - 1]->index + 1;

                pvec.nr = nr_pages;
                pagevec_release(&pvec);
                cond_resched();
        }
}


/*
 * Some rationalities:
 * 1. If a file is deleted by other clients w/o noticing us, it is OK since fid
 * never repeats, and the cached inode data will eventually be dropped out.
 * 2. If a file's content is changed by others, we need to watchout. We check
 * it by checking timestamps and i_size. Only trust cache when we can trust local
 * i_size and timestamps haven't changed.
 */
const struct fscache_cookie_def ll_fscache_inode_object_def = {
	.name		= "LUSTRE.inode",
	.type		= FSCACHE_COOKIE_TYPE_DATAFILE,
	.get_key	= ll_fscache_inode_get_key,
	.get_attr	= ll_fscache_inode_get_attr,
	.get_aux	= ll_fscache_inode_get_aux,
	.check_aux	= ll_fscache_inode_check_aux,
	.now_uncached	= ll_fscache_inode_now_uncached,
};
