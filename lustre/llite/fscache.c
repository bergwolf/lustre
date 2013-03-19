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

void ll_fscache_get_super_cookie(struct ll_sb_info *sbi)
{
	CDEBUG(D_SUPER, "getting sb fscache cookie\n");
	sbi->ll_fscache = fscache_acquire_cookie(ll_fscache_netfs.primary_index,
						 &ll_fscache_super_index_def,
						 sbi);
}

void ll_fscache_release_super_cookie(struct ll_sb_info *sbi)
{
	CDEBUG(D_SUPER, "releasing sb fscache cookie\n");
	fscache_relinquish_cookie(sbi->ll_fscache, 0);
	sbi->ll_fscache = NULL;
}

static void ll_fscache_enable_inode_cookie(struct inode *inode)
{
	struct ll_inode_info	*lli = ll_i2info(inode);
	struct ll_sb_info	*sbi = ll_i2sbi(inode);

	if (lli->lli_fscache)
		return;

	if (sbi->ll_flags & LL_SBI_ENABLE_FSCACHE) {
		CDEBUG(D_INODE, "enabling fscache for inode=%lu\n",
				inode->i_ino);
		lli->lli_fscache = fscache_acquire_cookie(sbi->ll_fscache,
					&ll_fscache_inode_object_def,
					lli);
	}
}

static void ll_fscache_disable_inode_cookie(struct inode *inode)
{
	struct ll_inode_info     *lli = ll_i2info(inode);

	if (lli->lli_fscache) {
		CDEBUG(D_INODE, "disabling fscache for inode=%lu\n",
				inode->i_ino);
		fscache_uncache_all_inode_pages(lli->lli_fscache, inode);
		fscache_relinquish_cookie(lli->lli_fscache, 1);
		lli->lli_fscache = NULL;
	}
}

void ll_fscache_set_inode_cookie(struct inode *inode, struct file *filp)
{
	if ((filp->f_flags & O_ACCMODE) != O_RDONLY)
		ll_fscache_disable_inode_cookie(inode);
	else
		ll_fscache_enable_inode_cookie(inode);
}

void ll_fscache_release_inode_cookie(struct inode *inode)
{
	struct ll_inode_info     *lli = ll_i2info(inode);

	if (lli->lli_fscache) {
		CDEBUG(D_INODE, "releasing fscache for inode=%lu\n",
				inode->i_ino);
		fscache_relinquish_cookie(lli->lli_fscache, 0);
		lli->lli_fscache = NULL;
	}
}

int ll_fscache_release_page(struct page *page, gfp_t gfp_mask)
{
	struct ll_inode_info *lli = ll_i2info(page->mapping->host);

	return fscache_maybe_release_page(lli->lli_fscache, page, gfp_mask);
}

static void ll_fscache_readpage_cb(struct page *page, void *ctx, int err)
{
	if (!err)
		SetPageUptodate(page);
	unlock_page(page);
}

int __ll_fscache_readpage(struct inode *inode, struct page *page)
{
	struct ll_inode_info *lli = ll_i2info(inode);
	int ret;

	ret = fscache_read_or_alloc_page(lli->lli_fscache, page,
					 ll_fscache_readpage_cb,
					 NULL, GFP_KERNEL);
	switch (ret) {
	case 0: /* page found in fscache, read submitted */
		return 0;
	case -ENOBUFS: /* page won't be cached */
	case -ENODATA: /* page not in cache */
		return 1;
	default:
		CERROR("Unknown error returned by fscache: %d\n", ret);
	}

	return ret;
}

int __ll_fscache_readpages(struct inode *inode, struct address_space *mapping,
			   struct list_head *pages, unsigned *nr_pages)
{
	struct ll_inode_info *lli = ll_i2info(inode);
	int ret;

	ret = fscache_read_or_alloc_pages(lli->lli_fscache, mapping, pages,
					  nr_pages, ll_fscache_readpage_cb,
					  NULL, mapping_gfp_mask(mapping));
	switch (ret) {
	case 0: /* read submitted for all pages */
		return 0;
	case -ENOBUFS: /* some pages are not cached and can't be */
	case -ENODATA: /* some pages are not cached */
		return 1;
	default:
		CERROR("Unknown error returned by fscache: %d\n", ret);
	}

	return ret;
}

void __ll_fscache_add_page(struct inode *inode, struct page *page)
{
	struct ll_inode_info *lli = ll_i2info(inode);

	if (fscache_write_page(lli->lli_fscache, page, GFP_KERNEL) != 0)
		fscache_uncache_page(lli->lli_fscache, page);
}

void __ll_fscache_invalidate_page(struct inode *inode, struct page *page)
{
	struct ll_inode_info *lli = ll_i2info(inode);

	fscache_wait_on_page_write(lli->lli_fscache, page);
	fscache_uncache_page(lli->lli_fscache, page);
}
