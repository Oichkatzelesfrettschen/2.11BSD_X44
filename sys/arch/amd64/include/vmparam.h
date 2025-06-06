/*-
 * SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 * Copyright (c) 1994 John S. Dyson
 * All rights reserved.
 * Copyright (c) 2003 Peter Wemm
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
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
 *	from: @(#)vmparam.h	5.9 (Berkeley) 5/12/91
 * $FreeBSD$
 */

#ifndef _AMD64_VMPARAM_H_
#define _AMD64_VMPARAM_H_

#define	USRSTACK        			(VM_MAXUSER_ADDRESS - PGSIZE)

#define	DMAP_MIN_ADDRESS			K4VADDR(PDIR_SLOT_DIRECT, 0, 0, 0)
#define	DMAP_MAX_ADDRESS			K4VADDR(PDIR_SLOT_DIRECT+1, 0, 0, 0)

#define	PHYS_TO_DMAP(x)				((x) | DMAP_MIN_ADDRESS)
#define	DMAP_TO_PHYS(x)				((x) & ~DMAP_MIN_ADDRESS)

#define	VM_MAXUSER_ADDRESS_LA57		(UVADDR(PDIR_SLOT_PTE, 0, 0, 0, 0))
#define	VM_MAXUSER_ADDRESS_LA48		(UVADDR(0, PDIR_SLOT_PTE, 0, 0, 0))

#define	VM_MAXUSER_ADDRESS			((vm_offset_t)(VM_MAXUSER_ADDRESS_LA57))
#define UPT_MIN_ADDRESS 			(KV4ADDR(PDIR_SLOT_PTE, PGSIZE * 3, 0, 0, 0))
#define UPT_MAX_ADDRESS				(KV4ADDR(PDIR_SLOT_PTE, PDIR_SLOT_PTE, PDIR_SLOT_PTE, PDIR_SLOT_PTE))
#define VM_MAX_ADDRESS          	((vm_offset_t)(UPT_MAX_ADDRESS))
#define	VM_MIN_KERNEL_ADDRESS       (KV4ADDR(PDIR_SLOT_KERN, 0, 0, 0))
#define	VM_MAX_KERNEL_ADDRESS       (KV4ADDR(PDIR_SLOT_APTE, PDIR_SLOT_APTE, PDIR_SLOT_APTE, PDIR_SLOT_APTE))

#endif /* _AMD64_VMPARAM_H_ */
