/*	$NetBSD: md4hl.c,v 1.7 2005/09/26 03:01:41 christos Exp $	*/

/*
 * Written by Jason R. Thorpe <thorpej@NetBSD.org>, April 29, 1997.
 * Public domain.
 */

#include <sys/cdefs.h>
#if defined(LIBC_SCCS) && !defined(lint)
__RCSID("$NetBSD: md4hl.c,v 1.7 2005/09/26 03:01:41 christos Exp $");
#endif /* LIBC_SCCS and not lint */

#include "namespace.h"

#if HAVE_NBTOOL_CONFIG_H
#include <md4.h>
#else
#include <hash/md4.h>
#endif

#define	MDALGORITHM	MD4

#if HAVE_NBTOOL_CONFIG_H
#define MDINCLUDE <md4.h>
#else
#define MDINCLUDE <hash/md4.h>
#endif

#include "mdXhl.c"
