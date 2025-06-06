
#include <sys/cdefs.h>
#if defined(LIBC_SCCS) && !defined(lint)
#if 0
static char sccsid[] = "@(#)gcvt.c	5.2 (Berkeley) 3/9/86";
#endif
#endif /* LIBC_SCCS and not lint */

#include "namespace.h"

#include <stdlib.h>

#ifdef __weak_alias
__weak_alias(gcvt, _gcvt)
#endif

/*
 * gcvt  - Floating output conversion to
 * minimal length string
 */

char *
gcvt(number, ndigit, buf)
	double number;
	int ndigit;
	char *buf;
{
	int sign, decpt;
	register char *p1, *p2;
	register int i;

	p1 = ecvt(number, ndigit, &decpt, &sign);
	p2 = buf;
	if (sign)
		*p2++ = '-';
	for (i = ndigit - 1; i > 0 && p1[i] == '0'; i--)
		ndigit--;
	if ((decpt >= 0 && decpt - ndigit > 4) || (decpt < 0 && decpt < -3)) { /* use E-style */
		decpt--;
		*p2++ = *p1++;
		*p2++ = '.';
		for (i = 1; i < ndigit; i++)
			*p2++ = *p1++;
		*p2++ = 'e';
		if (decpt < 0) {
			decpt = -decpt;
			*p2++ = '-';
		} else
			*p2++ = '+';
		*p2++ = decpt / 10 + '0';
		*p2++ = decpt % 10 + '0';
	} else {
		if (decpt <= 0) {
			if (*p1 != '0')
				*p2++ = '.';
			while (decpt < 0) {
				decpt++;
				*p2++ = '0';
			}
		}
		for (i = 1; i <= ndigit; i++) {
			*p2++ = *p1++;
			if (i == decpt)
				*p2++ = '.';
		}
		if (ndigit < decpt) {
			while (ndigit++ < decpt)
				*p2++ = '0';
			*p2++ = '.';
		}
	}
	if (p2[-1] == '.')
		p2--;
	*p2 = '\0';
	return (buf);
}
