/* $NetBSD: exec_coff.h,v 1.2 2000/06/04 16:24:02 mycroft Exp $ */

/*-
 * Copyright (C) 2000 SAITOH Masanobu.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _SYS_EXEC_COFF_H_
#define _SYS_EXEC_COFF_H_

#include <machine/coff_machdep.h>

/* COFF file header */
struct coff_filehdr {
	u_short	f_magic;	/* magic number */
	u_short	f_nscns;	/* # of sections */
	long	f_timdat;	/* timestamp */
	long	f_symptr;	/* file offset of symbol table */
	long	f_nsyms;	/* # of symbol table entries */
	u_short	f_opthdr;	/* size of optional header */
	u_short	f_flags;	/* flags */
};

/* f_flags */
#define COFF_F_RELFLG	0x1
#define COFF_F_EXEC		0x2
#define COFF_F_LNNO		0x4
#define COFF_F_LSYMS	0x8
#define COFF_F_SWABD	0x40
#define COFF_F_AR16WR	0x80
#define COFF_F_AR32WR	0x100
#define COFF_F_AR32W	0x200

/* COFF system header */
struct coff_aouthdr {
	short	a_magic;
	short	a_vstamp;
	long	a_tsize;
	long	a_dsize;
	long	a_bsize;
	long	a_entry;
	long	a_tstart;
	long	a_dstart;
};

/* COFF section header */
struct coff_scnhdr {
	char	s_name[8];
	long	s_paddr;
	long	s_vaddr;
	long	s_size;
	long	s_scnptr;
	long	s_relptr;
	long	s_lnnoptr;
	u_short	s_nreloc;
	u_short	s_nlnno;
	long	s_flags;
};

/* COFF shared library header */
struct coff_slhdr {
	long	sl_entry_len;	/* in words */
	long	sl_path_index;	/* in words */
	char	sl_name[1];
};

/* COFF symbol table */
struct coff_symtab {
	char	n_name[8];		/* symbol name */
	long	n_value;		/* value of symbol */
	short	n_scnum;		/* section number */
	u_short	n_type;			/* symbol type */
	char	n_sclass;		/* storage class */
	char	n_numaux;		/* auxiliary count */
};

/* COFF relocation entries */
struct coff_reloc {
	long	r_vaddr;		/* reference address */
	long	r_symndx;		/* symbol index */
	u_short	r_type;			/* type of relocation */
};

struct coff_exechdr {
	struct coff_filehdr f;
	struct coff_aouthdr a;
};

#define COFF_ROUND(val, by)     (((val) + by - 1) & ~(by - 1))

#define COFF_ALIGN(a) ((a) & ~(COFF_LDPGSZ - 1))

#define COFF_HDR_SIZE 														\
	(sizeof(struct coff_filehdr) + sizeof(struct coff_aouthdr))

#define COFF_BLOCK_ALIGN(ap, value) 										\
        ((ap)->a_magic == COFF_ZMAGIC ? COFF_ROUND(value, COFF_LDPGSZ) : 	\
         value)

#define COFF_TXTOFF(fp, ap) 												\
        ((ap)->a_magic == COFF_ZMAGIC ? 0 : 								\
         COFF_ROUND(COFF_HDR_SIZE + (fp)->f_nscns * 						\
		    sizeof(struct coff_scnhdr), 									\
		    COFF_SEGMENT_ALIGNMENT(fp, ap)))

#define COFF_DATOFF(fp, ap) 												\
        (COFF_BLOCK_ALIGN(ap, COFF_TXTOFF(fp, ap) + (ap)->a_tsize))

#define COFF_SEGMENT_ALIGN(fp, ap, value) 									\
        (COFF_ROUND(value, ((ap)->a_magic == COFF_ZMAGIC ? COFF_LDPGSZ : 	\
         COFF_SEGMENT_ALIGNMENT(fp, ap))))

#ifdef _KERNEL
struct exec_linker;

int	exec_coff_linker(struct exec_linker *);
int	exec_coff_prep_zmagic(struct exec_linker *, struct coff_filehdr *, struct coff_aouthdr *);
int	exec_coff_prep_nmagic(struct exec_linker *, struct coff_filehdr *, struct coff_aouthdr *);
int	exec_coff_prep_omagic(struct exec_linker *, struct coff_filehdr *, struct coff_aouthdr *);
#endif /* _KERNEL */
#endif /* _SYS_EXEC_COFF_H_ */
