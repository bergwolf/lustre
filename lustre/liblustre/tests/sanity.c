/* -*- mode: c; c-basic-offset: 8; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 * Lustre Light user test program
 *
 *  Copyright (c) 2002, 2003 Cluster File Systems, Inc.
 *
 *   This file is part of Lustre, http://www.lustre.org.
 *
 *   Lustre is free software; you can redistribute it and/or
 *   modify it under the terms of version 2 of the GNU General Public
 *   License as published by the Free Software Foundation.
 *
 *   Lustre is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Lustre; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/queue.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>

#include "test_common.h"

#define ENTRY(str)                                                      \
        do {                                                            \
                char buf[100];                                          \
                int len;                                                \
                sprintf(buf, "===== START: %s ", (str));                \
                len = strlen(buf);                                      \
                if (len < 79) {                                         \
                        memset(buf+len, '=', 100-len);                  \
                        buf[79] = '\n';                                 \
                        buf[80] = 0;                                    \
                }                                                       \
                printf("%s", buf);                                      \
        } while (0)

#define LEAVE()                                                         \
        do {                                                            \
                printf("----- END TEST successfully ---");              \
                printf("-----------------------------");                \
                printf("-------------------\n");                        \
        } while (0)

#define MAX_PATH_LENGTH 4096

void t1()
{
        char *path="/mnt/lustre/test_t1";
        ENTRY("create/delete");

        t_touch(path);
        t_unlink(path);
        LEAVE();
}

void t2()
{
        char *path="/mnt/lustre/test_t2";
        ENTRY("mkdir/rmdir");

        t_mkdir(path);
        t_rmdir(path);
        LEAVE();
}

void t3()
{
        char *path="/mnt/lustre/test_t3";
        ENTRY("regular stat");

        t_touch(path);
        t_check_stat(path, NULL);
        t_unlink(path);
        LEAVE();
}

void t4()
{
        char *path="/mnt/lustre/test_t4";
        ENTRY("dir stat");

        t_mkdir(path);
        t_check_stat(path, NULL);
        t_rmdir(path);
        LEAVE();
}

void t6()
{
        char *path="/mnt/lustre/test_t6";
        char *path2="/mnt/lustre/test_t6_link";
        ENTRY("symlink");

        t_touch(path);
        t_symlink(path, path2);
        t_check_stat(path2, NULL);
        t_unlink(path2);
        t_unlink(path);
        LEAVE();
}

void t7()
{
        char *path="/mnt/lustre/test_t7";
        int rc;
        ENTRY("mknod");

        if (geteuid() != 0) {
                rc = mknod(path, S_IFCHR | 0644, (5<<8 | 4));
                if (rc != -1 || errno != EPERM) {
                        printf("mknod shouldn't success: rc %d, errno %d\n",
                                rc, errno);
                }
        } else {
                t_mknod(path, S_IFCHR | 0644, 5, 4);
                t_check_stat(path, NULL);
                t_unlink(path);
        }
        LEAVE();
}

void t8()
{
        char *path="/mnt/lustre/test_t8";
        ENTRY("chmod");

        t_touch(path);
        t_chmod_raw(path, 0700);
        t_check_stat(path, NULL);
        t_unlink(path);
        LEAVE();
}

void t9()
{
        char *path="/mnt/lustre/test_t9";
        char *path2="/mnt/lustre/test_t9_link";
        ENTRY("hard link");

        t_touch(path);
        t_link(path, path2);
        t_check_stat(path, NULL);
        t_check_stat(path2, NULL);
        t_unlink(path);
        t_unlink(path2);
        LEAVE();
}

void t10()
{
        char *dir1="/mnt/lustre/test_t10_dir1";
        char *dir2="/mnt/lustre/test_t10_dir2";
        char *path1="/mnt/lustre/test_t10_reg1";
        char *path2="/mnt/lustre/test_t10_reg2";
        char *rename1="/mnt/lustre/test_t10_dir1/rename1";
        char *rename2="/mnt/lustre/test_t10_dir2/rename2";
        char *rename3="/mnt/lustre/test_t10_dir2/rename3";
        ENTRY("rename");

        t_mkdir(dir1);
        t_mkdir(dir2);
        t_touch(path1);
        t_touch(path2);
        t_rename(path1, rename1);
        t_rename(path2, rename2);
        t_rename(rename1, rename2);
        t_rename(dir1, rename3);
        t_unlink(rename2);
        t_rmdir(rename3);
        t_rmdir(dir2);
        LEAVE();
}

void t11()
{
        char *base="/mnt/lustre";
        char path[MAX_PATH_LENGTH], path2[MAX_PATH_LENGTH];
        int i, j, level = 5, nreg = 5;
        ENTRY("deep tree");

        safe_strncpy(path, base, MAX_PATH_LENGTH);

        for (i = 0; i < level; i++) {
                for (j = 0; j < nreg; j++) {
                        sprintf(path2, "%s/file%d", path, j);
                        t_touch(path2);
                }

                strcat(path, "/dir");
                t_mkdir(path);
        }

        for (i = level; i > 0; i--) {
                safe_strncpy(path, base, MAX_PATH_LENGTH);
                for (j = 1; j < i; j++)
                        strcat(path, "/dir");
                
                for (j = 0; j < nreg; j++) {
                        sprintf(path2, "%s/file%d", path, j);
                        t_unlink(path2);
                }

                strcat(path, "/dir");
                t_rmdir(path);
        }

        LEAVE();
}

void t12()
{
        char *dir="/mnt/lustre/test_t12_dir";
        char buf[1024*128];
        int fd;
        ENTRY("empty directory readdir");

        t_mkdir(dir);
        fd = t_opendir(dir);
        t_ls(fd, buf, sizeof(buf));
        t_close(fd);
        t_rmdir(dir);
        LEAVE();
}

void t13()
{
        char *dir="/mnt/lustre/test_t13_dir/";
        char name[1024];
        char buf[1024];
        const int nfiles = 20;
        char *prefix = "test13_filename_prefix_";
        int fd, i;
        ENTRY("multiple entries directory readdir");

        t_mkdir(dir);
        printf("Creating %d files...\n", nfiles);
        for (i = 0; i < nfiles; i++) {
                sprintf(name, "%s%s%05d", dir, prefix, i);
                t_touch(name);
        }
        fd = t_opendir(dir);
        t_ls(fd, buf, sizeof(buf));
        t_close(fd);
        printf("Cleanup...\n");
        for (i = 0; i < nfiles; i++) {
                sprintf(name, "%s%s%05d", dir, prefix, i);
                t_unlink(name);
        }
        t_rmdir(dir);
        LEAVE();
}

void t14()
{
        char *dir="/mnt/lustre/test_t14_dir/";
        char name[1024];
        char buf[1024];
        const int nfiles = 256;
        char *prefix = "test14_filename_long_prefix_AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA___";
	struct dirent64 *ent;
        int fd, i, rc, pos, index;
	loff_t base = 0;
        ENTRY(">1 block(4k) directory readdir");

        t_mkdir(dir);
        printf("Creating %d files...\n", nfiles);
        for (i = 0; i < nfiles; i++) {
                sprintf(name, "%s%s%05d", dir, prefix, i);
                t_touch(name);
        }
        fd = t_opendir(dir);
        printf("Listing...\n");
        index = 0;
	while ((rc = getdirentries64(fd, buf, 1024, &base)) > 0) {
		pos = 0;
		while (pos < rc) {
                        char *item;

			ent = (struct dirent64 *) ((char*) buf + pos);
                        item = (char *) ent->d_name;
                        if (!strcmp(item, ".") || !strcmp(item, ".."))
                                goto iter;
                        if (strstr(item, prefix) != item) {
                                printf("found bad name %s\n", item);
                                exit(-1);
                        }
			printf("[%03d]: %s\n",
                                index++, item + strlen(prefix));
iter:
			pos += ent->d_reclen;
		}
	}
	if (rc < 0) {
		printf("getdents error %d\n", rc);
                exit(-1);
	}
        if (index != nfiles) {
                printf("get %d files != %d\n", index, nfiles);
                exit(-1);
        }
        t_close(fd);
        printf("Cleanup...\n");
        for (i = 0; i < nfiles; i++) {
                sprintf(name, "%s%s%05d", dir, prefix, i);
                t_unlink(name);
        }
        t_rmdir(dir);
        LEAVE();
}

void t15()
{
        char *file = "/mnt/lustre/test_t15_file";
        int fd;
        ENTRY("open-stat-close");

        t_touch(file);
        fd = t_open(file);
        t_check_stat(file, NULL);
        t_close(fd);
        t_unlink(file);
        LEAVE();
}

void t16()
{
        char *file = "/mnt/lustre/test_t16_file";
        int fd;
        ENTRY("small-write-read");

        t_echo_create(file, "aaaaaaaaaaaaaaaaaaaaaa");
        t_grep(file, "aaaaaaaaaaaaaaaaaaaaaa");
        t_unlink(file);
        LEAVE();
}

void t17()
{
        char *file = "/mnt/lustre/test_t17_file";
        int fd;
        ENTRY("open-unlink without close");

        fd = open(file, O_WRONLY | O_CREAT, 0666);
        if (fd < 0) {
                printf("failed to create file: %s\n", strerror(errno));
                exit(-1);
        }
        t_unlink(file);
        LEAVE();
}

void t18()
{
        char *file = "/mnt/lustre/test_t18_file";
        char buf[128];
        int fd, i;
        struct stat statbuf[3];
        ENTRY("write should change mtime/atime");

        for (i = 0; i < 3; i++) {
                fd = open(file, O_RDWR|O_CREAT|O_APPEND, (mode_t)0666);
                if (fd < 0) {
                        printf("error open file: %s\n", strerror(errno));
                        exit(-1);
                }
                if (write(fd, buf, sizeof(buf)) != sizeof(buf)) {
                        printf("error write file\n");
                        exit(-1);
                }
                close(fd);
                if(stat(file, &statbuf[i]) != 0) {
                        printf("Error stat\n");
                        exit(1);
                }
                printf("mtime %ld, ctime %d\n",
                        statbuf[i].st_atime, statbuf[i].st_mtime);
                sleep(2);
        }

        for (i = 1; i < 3; i++) {
                if ((statbuf[i].st_atime <= statbuf[i-1].st_atime) ||
                    (statbuf[i].st_mtime <= statbuf[i-1].st_mtime)) {
                        printf("time error\n");
                        exit(-1);
                }
        }
        t_unlink(file);
}

#define PAGE_SIZE (4096)
#define _npages (2048)

static int _buffer[_npages][PAGE_SIZE/sizeof(int)];

/* pos:   i/o start from
 * xfer:  npages per transfer
 */
static void pages_io(int xfer, loff_t pos)
{
        char *path="/mnt/lustre/test_t50";
        int check_sum[_npages] = {0,};
        int fd, rc, i, j;
        struct timeval tw1, tw2, tr1, tr2;
        double tw, tr;

        memset(_buffer, 0, sizeof(_buffer));

        /* create sample data */
        for (i = 0; i < _npages; i++) {
                for (j = 0; j < PAGE_SIZE/sizeof(int); j++) {
                        _buffer[i][j] = rand();
                }
        }

        /* compute checksum */
        for (i = 0; i < _npages; i++) {
                for (j = 0; j < PAGE_SIZE/sizeof(int); j++) {
                        check_sum[i] += _buffer[i][j];
                }
        }

        t_touch(path);

	fd = t_open(path);

        /* write */
	lseek(fd, pos, SEEK_SET);
        gettimeofday(&tw1, NULL);
	for (i = 0; i < _npages; i += xfer) {
		rc = write(fd, _buffer[i], PAGE_SIZE * xfer);
                if (rc != PAGE_SIZE * xfer) {
                        printf("write error %d (i = %d)\n", rc, i);
                        exit(1);
                }
	}
        gettimeofday(&tw2, NULL);

        memset(_buffer, 0, sizeof(_buffer));

        /* read */
	lseek(fd, pos, SEEK_SET);
        gettimeofday(&tr1, NULL);
	for (i = 0; i < _npages; i += xfer) {
		rc = read(fd, _buffer[i], PAGE_SIZE * xfer);
                if (rc != PAGE_SIZE * xfer) {
                        printf("read error %d (i = %d)\n", rc, i);
                        exit(1);
                }
	}
        gettimeofday(&tr2, NULL);

        /* compute checksum */
        for (i = 0; i < _npages; i++) {
                int sum = 0;
                for (j = 0; j < PAGE_SIZE/sizeof(int); j++) {
                        sum += _buffer[i][j];
                }
                if (sum != check_sum[i]) {
                        printf("chunk %d checksum error: expected 0x%x, get 0x%x\n",
                                i, check_sum[i], sum);
                }
        }

	t_close(fd);
        t_unlink(path);
        tw = (tw2.tv_sec - tw1.tv_sec) * 1000000 + (tw2.tv_usec - tw1.tv_usec);
        tr = (tr2.tv_sec - tr1.tv_sec) * 1000000 + (tr2.tv_usec - tr1.tv_usec);
        printf(" (R:%.3fM/s, W:%.3fM/s)\n",
                (_npages * PAGE_SIZE) / (tw / 1000000.0) / (1024 * 1024),
                (_npages * PAGE_SIZE) / (tr / 1000000.0) / (1024 * 1024));
}

void t50()
{
        char text[256];
        loff_t off_array[] = {1, 17, 255, 258, 4095, 4097, 8191, 1024*1024*1024*1024ULL};
        int np = 1, i;
        loff_t offset = 0;

        ENTRY("4k aligned i/o sanity");
        while (np <= _npages) {
                printf("%3d per xfer(total %d)...\t", np, _npages);
                pages_io(np, offset);
                np += np;
        }
        LEAVE();

        ENTRY("4k un-aligned i/o sanity");
        for (i = 0; i < sizeof(off_array)/sizeof(loff_t); i++) {
                offset = off_array[i];
                printf("16 per xfer(total %d), offset %10lld...\t",
                        _npages, offset);
                pages_io(16, offset);
        }
        LEAVE();
}

extern void __liblustre_setup_(void);
extern void __liblustre_cleanup_(void);

void usage(char *cmd)
{
        printf("Usage: \t%s --target mdsnid:/mdsname/profile\n", cmd);
        printf("       \t%s --dumpfile dumpfile\n", cmd);
        exit(-1);
}

int main(int argc, char * const argv[])
{
        int opt_index, c;
        static struct option long_opts[] = {
                {"target", 1, 0, 0},
                {"dumpfile", 1, 0, 0},
                {0, 0, 0, 0}
        };

        if (argc <= 1)
                usage(argv[0]);

        while ((c = getopt_long(argc, argv, "", long_opts, &opt_index)) != -1) {
                switch (c) {
                case 0: {
                        if (!optarg[0])
                                usage(argv[0]);

                        if (!strcmp(long_opts[opt_index].name, "target")) {
                                setenv(ENV_LUSTRE_MNTTGT, optarg, 1);
                        } else if (!strcmp(long_opts[opt_index].name, "dumpfile")) {
                                setenv(ENV_LUSTRE_DUMPFILE, optarg, 1);
                        } else
                                usage(argv[0]);
                        break;
                }
                default:
                        usage(argv[0]);
                }
        }

        if (optind != argc)
                usage(argv[0]);

        __liblustre_setup_();

        t1();
        t2();
        t3();
        t4();
        t6();
        t7();
        t8();
        t9();
        t10();
        t11();
        t12();
        t13();
        t14();
        t15();
        t16();
        t17();
        t18();
        t50();

	printf("liblustre is about shutdown\n");
        __liblustre_cleanup_();

	printf("complete successfully\n");
	return 0;
}
