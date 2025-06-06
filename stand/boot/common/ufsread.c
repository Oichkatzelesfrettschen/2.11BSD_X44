/*-
 * Copyright (c) 2002 Networks Associates Technology, Inc.
 * All rights reserved.
 *
 * This software was developed for the FreeBSD Project by Marshall
 * Kirk McKusick and Network Associates Laboratories, the Security
 * Research Division of Network Associates, Inc. under DARPA/SPAWAR
 * contract N66001-01-C-8035 ("CBOSS"), as part of the DARPA CHATS
 * research program
 *
 * Copyright (c) 1998 Robert Nordier
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are freely
 * permitted provided that the above copyright notice and this
 * paragraph and the following disclaimer are duplicated in all
 * such forms.
 *
 * This software is provided "AS IS" and without any express or
 * implied warranties, including, without limitation, the implied
 * warranties of merchantability and fitness for a particular
 * purpose.
 *
 * $FreeBSD: src/sys/boot/common/ufsread.c,v 1.12 2003/08/25 23:30:41 obrien Exp $
 * $DragonFly: src/sys/boot/common/ufsread.c,v 1.5 2008/09/13 11:46:28 corecode Exp $
 */

#include <sys/cdefs.h>
/* __FBSDID("$FreeBSD$"); */

#ifdef BOOT2
#include "boot2.h"
#else
#include <sys/param.h>
#endif
#include <sys/user.h>
#include <sys/disklabel.h>
#include <sys/dirent.h>

#include <ufs/ufs/dinode.h>
#include <ufs/ufs/dir.h>
#include <ufs/ffs/fs.h>

#if defined(__i386__)
/* XXX: Revert to old (broken for over 1.5Tb filesystems) version of cgbase
   (see sys/ufs/ffs/fs.h rev 1.39) so that i386 boot loader (boot2) can
   support both UFS1 and UFS2 again. */
#undef 	cgbase
#define cgbase(fs, c)   ((ufs2_daddr_t)((fs)->fs_fpg * (c)))
#endif
/*
#define UFS_MAXNAMLEN			MAXNAMLEN
#define UFS_ROOTINO				ROOTINO
#define UFS_NDADDR				NDADDR
#define SBLOCKSIZE				SBSIZE
*/

/*
 * We use 4k `virtual' blocks for filesystem data, whatever the actual
 * filesystem block size. FFS blocks are always a multiple of 4k.
 */
#define VBLKSHIFT				12
#define VBLKSIZE				(1 << VBLKSHIFT)
#define VBLKMASK				(VBLKSIZE - 1)
#define DBPERVBLK				(VBLKSIZE / DEV_BSIZE)
#define INDIRPERVBLK(fs) 		(NINDIR(fs) / ((fs)->fs_bsize >> VBLKSHIFT))
#define IPERVBLK(fs)			(INOPB(fs) / ((fs)->fs_bsize >> VBLKSHIFT))
#define INO_TO_VBO(ipervblk, x) ((x) % ipervblk)
#define INO_TO_VBA(fs, ipervblk, x)					\
	(fsbtodb(fs, cgimin(fs, ino_to_cg(fs, x))) + 	\
	(((x) % (fs)->fs_ipg) / (ipervblk) * DBPERVBLK))

#define FS_TO_VBA(fs, fsb, off) (fsbtodb(fs, fsb) + \
    ((off) / VBLKSIZE) * DBPERVBLK)
#define FS_TO_VBO(fs, fsb, off) ((off) & VBLKMASK)

/* Buffers that must not span a 64k boundary. */
struct ufs_dmadat {
#ifdef BOOT2
	struct boot2_dmadat boot2;
#else
	char secbuf[DEV_BSIZE*4];	/* for MBR/disklabel */
#endif
	char blkbuf[VBLKSIZE];		/* filesystem blocks */
	char indbuf[VBLKSIZE];		/* indir blocks */
	char sbbuf[SBSIZE];			/* superblock */
};
static struct ufs_dmadat *dmadat;
#define fsdmadat	((struct ufs_dmadat *)boot2_dmadat)

static uint32_t 	boot_lookup(const char *);
static ssize_t 		boot_fsread(uint32_t, void *, size_t);

#ifdef BOOT2
const struct boot2_fsapi boot2_ufs_api = {
		.fsinit = boot_init,
		.fslookup = boot_lookup,
		.fsread = boot_fsread
};
#endif

static uint8_t ls, dsk_meta;
static uint32_t fs_off, inomap;

static __inline uint8_t
fsfind(const char *name, uint32_t * ino)
{
	static char buf[DEV_BSIZE];
	static struct direct d;
	char *s;
	ssize_t n;

	fs_off = 0;
	while ((n = fsread(*ino, buf, DEV_BSIZE)) > 0)
		for (s = buf; s < buf + DEV_BSIZE;) {
			memcpy(&d, s, sizeof(struct direct));
			if (ls)
				printf("%s ", d.d_name);
			else if (!strcmp(name, d.d_name)) {
				*ino = d.d_ino;
				return d.d_type;
			}
			s += d.d_reclen;
		}
	if (n != -1 && ls)
		printf("\n");
	return 0;
}

uint32_t
boot_lookup(const char *path)
{
	static char name[UFS_MAXNAMLEN + 1];
	const char *s;
	uint32_t ino;
	ssize_t n;
	uint8_t dt;

	ino = UFS_ROOTINO;
	dt = DT_DIR;
	for (;;) {
		if (*path == '/')
			path++;
		if (!*path)
			break;
		for (s = path; *s && *s != '/'; s++)
			;
		if ((n = s - path) > UFS_MAXNAMLEN)
			return 0;
		ls = *path == '?' && n == 1 && !*s;
		memcpy(name, path, n);
		name[n] = 0;
		if (dt != DT_DIR) {
			printf("%s: not a directory.\n", name);
			return (0);
		}
		if ((dt = fsfind(name, &ino)) <= 0)
			break;
		path = s;
	}
	return dt == DT_REG ? ino : 0;
}

/*
 * Possible superblock locations ordered from most to least likely.
 */
static int sblock_try[] = SBLOCKSEARCH;

#if defined(UFS2_ONLY)
#define DIP(field) dp2.field
#elif defined(UFS1_ONLY)
#define DIP(field) dp1.field
#else
#define DIP(field) fs.fs_magic == FS_UFS1_MAGIC ? dp1.field : dp2.field
#endif

int
boot_init(void)
{
	static struct fs fs;
	size_t n;

	inomap = 0;
	fs = (struct fs *)fsdmadat->sbbuf;

	for (n = 0; sblock_try[n] != -1; n++) {
		if (dskread(dmadat->sbbuf, sblock_try[n] / DEV_BSIZE,
				SBLOCKSIZE / DEV_BSIZE))
			return -1;
		memcpy(&fs, dmadat->sbbuf, sizeof(struct fs));
		if ((
#if defined(UFS1_ONLY)
		    fs.fs_magic == FS_UFS1_MAGIC
#elif defined(UFS2_ONLY)
		    (fs.fs_magic == FS_UFS2_MAGIC &&
		    		fs.fs_sblockloc == sblock_try[n])
#else
				fs.fs_magic == FS_UFS1_MAGIC
						|| (fs.fs_magic == FS_UFS2_MAGIC
								&& fs.fs_sblockloc == sblock_try[n])
#endif
				) && fs.fs_bsize <= MAXBSIZE
				&& fs.fs_bsize >= (int32_t) sizeof(struct fs))
			break;
	}
	if (sblock_try[n] == -1) {
		return -1;
	}
	return (0);
}

static ssize_t
fsread_size(uint32_t inode, void *buf, size_t nbyte, size_t *fsizep)
{
#ifndef UFS2_ONLY
	static struct ufs1_dinode dp1;
	ufs1_daddr_t addr1;
#endif
#ifndef UFS1_ONLY
	static struct ufs2_dinode dp2;
#endif
	static struct fs fs;
	char *blkbuf;
	void *indbuf;
	char *s;
	size_t n, nb, size, off, vboff;
	ufs_lbn_t lbn;
	ufs2_daddr_t addr2, vbaddr;
	static ufs2_daddr_t blkmap, indmap;
	u_int u;

	/* Basic parameter validation. */
	if ((buf == NULL && nbyte != 0) || dmadat == NULL)
		return (-1);

	blkbuf = dmadat->blkbuf;
	indbuf = dmadat->indbuf;
	fs = (struct fs *)fsdmadat->sbbuf;

#ifndef BOOT2
	/*
	 * Force probe if inode is zero to ensure we have a valid fs, otherwise
	 * when probing multiple paritions, reads from subsequent parititions
	 * will incorrectly succeed.
	 */
	if (!dsk_meta || inode == 0) {
		inomap = 0;
		dsk_meta = 0;
		if (boot_init() == 0) {
			dsk_meta++;
		}
	}
#endif
	else {
		memcpy(&fs, dmadat->sbbuf, sizeof(struct fs));
	}
	if (!inode) {
		return 0;
	}
#ifndef BOOT2
	else if (!dsk_meta) {
		return -1;
	}
#endif
	if (inomap != inode) {
		n = IPERVBLK(&fs);
		if (dskread(blkbuf, INO_TO_VBA(&fs, n, inode), DBPERVBLK))
			return -1;
		n = INO_TO_VBO(n, inode);
#if defined(UFS1_ONLY)
		memcpy(&dp1, (struct ufs1_dinode *)(void *)blkbuf + n, sizeof(dp1));
#elif defined(UFS2_ONLY)
		memcpy(&dp2, (struct ufs2_dinode *)(void *)blkbuf + n, sizeof(dp2));
#else
		if (fs.fs_magic == FS_UFS1_MAGIC)
			memcpy(&dp1, (struct ufs1_dinode *)(void *)blkbuf + n, sizeof(dp1));
		else
			memcpy(&dp2, (struct ufs2_dinode *)(void *)blkbuf + n, sizeof(dp2));
#endif
		inomap = inode;
		fs_off = 0;
		blkmap = indmap = 0;
	}
	s = buf;
	size = DIP(di_size);
	n = size - fs_off;
	if (nbyte > n)
		nbyte = n;
	nb = nbyte;
	while (nb) {
		lbn = lblkno(&fs, fs_off);
		off = blkoff(&fs, fs_off);
		if (lbn < UFS_NDADDR) {
			addr2 = DIP(di_db[lbn]);
		} else if (lbn < UFS_NDADDR + NINDIR(&fs)) {
			n = INDIRPERVBLK(&fs);
			addr2 = DIP(di_ib[0]);
			u = (u_int) (lbn - UFS_NDADDR) / n * DBPERVBLK;
			vbaddr = fsbtodb(&fs, addr2) + u;
			if (indmap != vbaddr) {
				if (dskread(indbuf, vbaddr, DBPERVBLK))
					return -1;
				indmap = vbaddr;
			}
			n = (lbn - UFS_NDADDR) & (n - 1);
#if defined(UFS1_ONLY)
			memcpy(&addr1, (ufs1_daddr_t *)indbuf + n,
			    sizeof(ufs1_daddr_t));
			addr2 = addr1;
#elif defined(UFS2_ONLY)
			memcpy(&addr2, (ufs2_daddr_t *)indbuf + n,
			    sizeof(ufs2_daddr_t));
#else
			if (fs.fs_magic == FS_UFS1_MAGIC) {
				memcpy(&addr1, (ufs1_daddr_t*) indbuf + n,
						sizeof(ufs1_daddr_t));
				addr2 = addr1;
			} else
				memcpy(&addr2, (ufs2_daddr_t*) indbuf + n,
						sizeof(ufs2_daddr_t));
#endif
		} else
			return -1;
		vbaddr = fsbtodb(&fs, addr2) + (off >> VBLKSHIFT) * DBPERVBLK;
		vboff = off & VBLKMASK;
		n = sblksize(&fs, (off_t) size, lbn) - (off & ~VBLKMASK);
		if (n > VBLKSIZE)
			n = VBLKSIZE;
		if (blkmap != vbaddr) {
			if (dskread(blkbuf, vbaddr, n >> DEV_BSHIFT))
				return -1;
			blkmap = vbaddr;
		}
		n -= vboff;
		if (n > nb)
			n = nb;
		memcpy(s, blkbuf + vboff, n);
		s += n;
		fs_off += n;
		nb -= n;
	}

	if (fsizep != NULL)
		*fsizep = size;

	return (nbyte);
}

ssize_t
boot_fsread(uint32_t inode, void *buf, size_t nbyte)
{
	return (fsread_size(inode, buf, nbyte, NULL));
}
