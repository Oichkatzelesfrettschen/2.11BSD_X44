/*	$NetBSD: bpfdesc.h,v 1.16.2.1 2004/04/21 03:56:20 jmc Exp $	*/

/*
 * Copyright (c) 1990, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from the Stanford/CMU enet packet filter,
 * (net/enet.c) distributed as part of 4.3BSD, and code contributed
 * to Berkeley by Steven McCanne and Van Jacobson both of Lawrence
 * Berkeley Laboratory.
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
 *	@(#)bpfdesc.h	8.1 (Berkeley) 6/10/93
 *
 * @(#) Header: bpfdesc.h,v 1.14 96/06/16 22:28:07 leres Exp  (LBL)
 */

#ifndef _NET_BPFDESC_H_
#define _NET_BPFDESC_H_

#include <sys/callout.h>
#include <sys/select.h>

/*
 * Descriptor associated with each open bpf file.
 */
struct bpf_d {
	struct bpf_d	*bd_next;	/* Linked list of descriptors */
	/*
	 * Buffer slots: two mbuf clusters buffer the incoming packets.
	 *   The model has three slots.  Sbuf is always occupied.
	 *   sbuf (store) - Receive interrupt puts packets here.
	 *   hbuf (hold) - When sbuf is full, put cluster here and
	 *                 wakeup read (replace sbuf with fbuf).
	 *   fbuf (free) - When read is done, put cluster here.
	 * On receiving, if sbuf is full and fbuf is 0, packet is dropped.
	 */
	caddr_t			bd_sbuf;	/* store slot */
	caddr_t			bd_hbuf;	/* hold slot */
	caddr_t			bd_fbuf;	/* free slot */
	int 			bd_slen;	/* current length of store buffer */
	int 			bd_hlen;	/* current length of hold buffer */

	int				bd_bufsize;	/* absolute length of buffers */

	struct bpf_if 	*bd_bif;		/* interface descriptor */
	u_long			bd_rtout;	/* Read timeout in 'ticks' */
	struct bpf_insn *bd_filter; 	/* filter code */
	u_long			bd_rcount;	/* number of packets received */
	u_long			bd_dcount;	/* number of packets dropped */
	u_long			bd_ccount;	/* number of packets captured */

	u_char			bd_promisc;	/* true if listening promiscuously */
	u_char			bd_state;	/* idle, waiting, or timed out */
	u_char			bd_immediate;	/* true to return on packet arrival */
	int				bd_hdrcmplt;	/* false to fill in src lladdr */
	int				bd_seesent;	/* true if bpf should see sent packets */
	int				bd_async;	/* non-zero if packet reception should generate signal */
	pid_t			bd_pgid;	/* process or group id for signal */
	struct file		*bd_file;	/* bpf file */
	u_char			bd_selcoll;	/* true if selects collide */
	int				bd_timedout;
	struct proc 	*bd_selproc;/* process that last selected us */
	u_char			bd_pad;		/* explicit alignment */
	struct selinfo	bd_sel;		/* bsd select info */
	struct callout	bd_callout;	/* for BPF timeouts with select */

	//struct nbpf_d  	*bd_nbpf;    /* nbpf extension structure */
};

/* Values for bd_state */
#define BPF_IDLE		0		/* no select in progress */
#define BPF_WAITING		1		/* waiting for read timeout in select */
#define BPF_TIMED_OUT	2		/* read timeout has expired in select */

/*
 * Descriptor associated with each attached hardware interface.
 */
struct bpf_if {
	struct bpf_if 	*bif_next;		/* list of all interfaces */
	struct bpf_d 	*bif_dlist;		/* descriptor list */
	struct bpf_if 	**bif_driverp;	/* pointer into softc */
	u_int 			bif_dlt;		/* link layer type */
	u_int 			bif_hdrlen;		/* length of header (with padding) */
	struct ifnet 	*bif_ifp;		/* correspoding interface */
};

#ifdef _KERNEL
int	 bpf_setf(struct bpf_d *, struct bpf_program *);
#endif

#endif /* _NET_BPFDESC_H_ */
