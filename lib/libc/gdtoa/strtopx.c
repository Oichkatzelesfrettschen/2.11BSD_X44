/* $NetBSD: strtopx.c,v 1.4 2008/03/21 23:13:48 christos Exp $ */

/****************************************************************

The author of this software is David M. Gay.

Copyright (C) 1998, 2000 by Lucent Technologies
All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name of Lucent or any of its entities
not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.

****************************************************************/

/* Please send bug reports to David M. Gay (dmg at acm dot org,
 * with " at " changed at "@" and " dot " changed to ".").	*/

#include "gdtoaimp.h"

#undef _0
#undef _1

/* one or the other of IEEE_BIG_ENDIAN or IEEE_LITTLE_ENDIAN should be #defined */

#ifdef IEEE_BIG_ENDIAN
#define _0 0
#define _1 1
#define _2 2
#define _3 3
#define _4 4
#endif
#ifdef IEEE_LITTLE_ENDIAN
#define _0 4
#define _1 3
#define _2 2
#define _3 1
#define _4 0
#endif

int
#ifdef KR_headers
strtopx(s, sp, V)
 	 CONST char *s; char **sp; void *V;
#else
strtopx(CONST char *s, char **sp, void *V)
#endif
{
	static const FPI fpi = { 64, 1-16383-64+1, 32766 - 16383 - 64 + 1, 1, SI };
	ULong bits[2];
	Long expt;
	int k;
	UShort *L = (UShort*)V;

	k = strtodg(s, sp, &fpi, &expt, bits);
	if (k == STRTOG_NoMemory)
		return k;
	switch (k & STRTOG_Retmask) {
	case STRTOG_NoNumber:
	case STRTOG_Zero:
		L[0] = L[1] = L[2] = L[3] = L[4] = 0;
		break;

	case STRTOG_Denormal:
		L[_0] = 0;
		goto normal_bits;

	case STRTOG_Normal:
	case STRTOG_NaNbits:
		L[_0] = expt + 0x3fff + 63;
 normal_bits:
		L[_4] = (UShort) bits[0];
		L[_3] = (UShort) (bits[0] >> 16);
		L[_2] = (UShort) bits[1];
		L[_1] = (UShort) (bits[1] >> 16);
		break;

	case STRTOG_Infinite:
		L[_0] = 0x7fff;
		L[_1] = 0x8000;
		L[_2] = L[_3] = L[_4] = 0;
		break;

	case STRTOG_NaN:
		L[0] = ldus_QNAN0;
		L[1] = ldus_QNAN1;
		L[2] = ldus_QNAN2;
		L[3] = ldus_QNAN3;
		L[4] = ldus_QNAN4;
	}
	if (k & STRTOG_Neg)
		L[_0] |= 0x8000;
	return k;
}
