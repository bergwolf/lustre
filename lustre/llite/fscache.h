/*
 * Copyright (c) 2013, EMC Corporation.
 *
 * This file is part of Lustre, http://www.lustre.org/
 *
 * Implementation of fscache hooks for llite.
 *
 *   Author: Peng Tao <bergwolf@gmail.com>
 */
#ifndef _LUSTRE_FSCACHE_H
#define _LUSTRE_FSCACHE_H
#include <linux/fscache.h>

#ifdef CONFIG_LUSTRE_FSCACHE
extern struct fscache_netfs ll_fscache_netfs;
extern const struct fscache_cookie_def ll_fscache_server_index_def;
extern const struct fscache_cookie_def ll_fscache_super_index_def;
extern const struct fscache_cookie_def ll_fscache_inode_object_def;

int ll_fscache_register(void);
void ll_fscache_unregister(void);

void ll_fscache_get_super_cookie(struct ll_sb_info *sbi);
void ll_fscache_release_super_cookie(struct ll_sb_info *sbi);
void ll_fscache_set_inode_cookie(struct inode *inode, struct file *filp);
void ll_fscache_release_inode_cookie(struct inode *inode);
int ll_fscache_release_page(struct page *page, gfp_t gfp_mask);
int __ll_fscache_readpage(struct inode *inode, struct page *page);
int __ll_fscache_readpages(struct inode *inode, struct address_space *mapping,
			   struct list_head *pages, unsigned *nr_pages);
void __ll_fscache_add_page(struct inode *inode, struct page *page);
void __ll_fscache_invalidate_page(struct inode *inode, struct page *page);

static inline int
ll_fscache_readpage(struct inode *inode, struct page *page)
{
	if (ll_i2info(inode)->lli_fscache)
		return __ll_fscache_readpage(inode, page);
	return -ENOBUFS;
}

static inline int
ll_fscache_readpages(struct inode *inode, struct address_space *mapping,
		     struct list_head *pages, unsigned *nr_pages)
{
	if (ll_i2info(inode)->lli_fscache)
		return __ll_fscache_readpages(inode, mapping, pages, nr_pages);
	return -ENOBUFS;
}

static inline void
ll_fscache_add_page(struct inode *inode, struct page *page)
{
	if (ll_i2info(inode)->lli_fscache)
		__ll_fscache_add_page(inode, page);
}

static inline void
ll_fscache_invalidate_page(struct inode *inode, struct page *page)
{
	if (ll_i2info(inode)->lli_fscache)
		__ll_fscache_invalidate_page(inode, page);
}

#else /* CONFIG_LUSTRE_FSCACHE */
static inline int ll_fscache_register(void) { return 0; }
static inline void ll_fscache_unregister(void) {}

static inline void
ll_fscache_get_super_cookie(struct ll_sb_info *sbi) {}
static inline void
ll_fscache_release_super_cookie(struct ll_sb_info *sbi) {}
static inline void
ll_fscache_set_inode_cookie(struct inode *inode, struct file *filp) {}
static inline void
ll_fscache_release_inode_cookie(struct inode *inode) {}
static inline int
ll_fscache_release_page(struct page *page, gfp_t gfp_mask) { return 0; }
static inline int
ll_fscache_readpage(struct inode *inode, struct page *page)
{
	return -ENOBUFS;
}
static inline int
ll_fscache_readpages(struct inode *inode, struct address_space *mapping,
		     struct list_head *pages, unsigned *nr_pages)
{
	return -ENOBUFS;
}
static inline void
ll_fscache_add_page(struct inode *inode, struct page *page) {}
static inline void
ll_fscache_invalidate_page(struct inode *inode, struct page *page) {}

#endif /* !CONFIG_LUSTRE_FSCACHE */

#endif /* _LUSTRE_FSCACHE_H */
