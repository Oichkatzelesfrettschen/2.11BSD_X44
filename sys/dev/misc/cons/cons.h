/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: @(#)cons.h	7.2 (Berkeley) 5/9/91
 * $FreeBSD$
 */
/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)cons.h	8.1 (Berkeley) 6/11/93
 */

#ifndef _SYS_DEV_CONS_H_
#define _SYS_DEV_CONS_H_

struct consdev {
	void		(*cn_probe)(struct consdev *);			/* probe hardware and fill in consdev info */
	void		(*cn_init)(struct consdev *);			/* turn on as console */
	int			(*cn_getc)(dev_t);						/* kernel getchar interface */
	void		(*cn_putc)(dev_t, int);					/* kernel putchar interface */
	void		(*cn_pollc)(dev_t, int); 				/* turn on and off polling */
	void		(*cn_bell)(dev_t, u_int, u_int, u_int);	/* ring bell */
	void		(*cn_halt)(dev_t);						/* stop device */
	void		(*cn_flush)(dev_t);						/* flush output */
	struct tty 	*cn_tp;									/* tty structure for console device */
	dev_t		cn_dev;									/* major/minor of device */
	int			cn_pri;									/* pecking order; the higher the better */
	int			cn_flags;								/* capabilities of this console */
	char		cn_name[256];					/* console (device) name */
};

/* values for cn_pri - reflect our policy for console selection */
#define	CN_DEAD			0	/* device doesn't exist */
#define CN_NULL			1	/* noop console */
#define CN_NORMAL		2	/* device exists but is nothing special */
#define CN_INTERNAL		3	/* "internal" bit-mapped display */
#define CN_REMOTE		4	/* serial interface with remote bit set */

/* Values for cn_flags. */
#define	CN_FLAG_NODEBUG	0x00000001	/* Not supported with debugger. */
#define	CN_FLAG_NOAVAIL	0x00000002	/* Temporarily not available. */

#ifdef _KERNEL
extern struct consdev *cn_tab;

/* console-specific types */
typedef void 	dev_type_cnprobe_t(struct consdev *);
typedef void 	dev_type_cninit_t(struct consdev *);
typedef int 	dev_type_cngetc_t(dev_t);
typedef void 	dev_type_cnputc_t(dev_t, int);
typedef void 	dev_type_cnpollc_t(dev_t, int);
typedef void 	dev_type_cnbell_t(dev_t, u_int, u_int, u_int);
typedef void 	dev_type_cnhalt_t(dev_t);
typedef void 	dev_type_cnflush_t(dev_t);

#define	dev_type_cnprobe(n)	dev_type_cnprobe_t n
#define	dev_type_cninit(n)	dev_type_cninit_t n
#define	dev_type_cngetc(n)	dev_type_cngetc_t n
#define	dev_type_cnputc(n)	dev_type_cnputc_t n
#define	dev_type_cnpollc(n)	dev_type_cnpollc_t n
#define	dev_type_cnbell(n)	dev_type_cnbell_t n
#define	dev_type_cnhalt(n)	dev_type_cnhalt_t n
#define	dev_type_cnflush(n)	dev_type_cnflush_t n

/* no dev routines */
#define nocnprobe	((dev_type_cnprobe_t *)enodev)
#define nocninit	((dev_type_cninit_t *)enodev)
#define nocngetc	((dev_type_cngetc_t *)enodev)
#define nocnputc	((dev_type_cnputc_t *)enodev)
#define nocnpollc	((dev_type_cnpollc_t *)enodev)
#define nocnbell	((dev_type_cnbell_t *)enodev)
#define nocnhalt	((dev_type_cnhalt_t *)enodev)
#define nocnflush	((dev_type_cnflush_t *)enodev)

/* null dev routines */
#define nullcnprobe	((dev_type_cnprobe_t *)nullop)
#define nullcninit	((dev_type_cninit_t *)nullop)
#define nullcngetc	((dev_type_cngetc_t *)nullop)
#define nullcnputc	((dev_type_cnputc_t *)nullop)
#define nullcnpollc	((dev_type_cnpollc_t *)nullop)
#define nullcnbell	((dev_type_cnbell_t *)nullop)
#define nullcnhalt	((dev_type_cnhalt_t *)nullop)
#define nullcnflush	((dev_type_cnflush_t *)nullop)

#endif /* KERNEL */

/* Other kernel entry points. */
void	cninit(void);
void	cninit_finish(void);
int		cnadd(struct consdev *);
void	cnavailable(struct consdev *, int);
int		cnunavailable(void);
void	cnremove(struct consdev *);
void	cnselect(struct consdev *);
int		cngetc(void);
int		cngetsn(char *, int);
void	cnputc(int);
void	cnpollc(int);
void	cnbell(u_int, u_int, u_int);
void	cnflush(void);
void	cnhalt(void);

#endif /* _SYS_DEV_CONS_H_ */
