/*	$NetBSD: nsdispatch.c,v 1.18 2002/05/26 14:48:19 wiz Exp $	*/

/*-
 * Copyright (c) 1997, 1998, 1999 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Luke Mewburn.
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
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#if defined(LIBC_SCCS) && !defined(lint)
__RCSID("$NetBSD: nsdispatch.c,v 1.18 2002/05/26 14:48:19 wiz Exp $");
#endif /* LIBC_SCCS and not lint */

#include "namespace.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <assert.h>
#include <err.h>
#include <fcntl.h>
#define _NS_PRIVATE
#include <nsswitch.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern	FILE 	*_nsyyin;
extern	int	 _nsyyparse(void);


#ifdef __weak_alias
__weak_alias(nsdispatch,_nsdispatch)
#endif

/*
 * default sourcelist: `files'
 */
const ns_src __nsdefaultsrc[] = {
		{ NSSRC_FILES, NS_SUCCESS },
		{ 0 },
};

static	int	_nsmapsize = 0;
static	ns_dbt	*_nsmap = NULL;

/*
 * size of dynamic array chunk for _nsmap and _nsmap[x].srclist
 */
#define NSELEMSPERCHUNK		8

int	_nscmp(const void *, const void *);

int
_nscmp(a, b)
	const void *a;
	const void *b;
{
	return (strcasecmp(((const ns_dbt*) a)->name, ((const ns_dbt*) b)->name));
}


int
_nsdbtaddsrc(dbt, src)
	ns_dbt		*dbt;
	const ns_src	*src;
{
	_DIAGASSERT(dbt != NULL);
	_DIAGASSERT(src != NULL);

	if ((dbt->srclistsize % NSELEMSPERCHUNK) == 0) {
		ns_src *new;

		new = (ns_src*) realloc(dbt->srclist,
				(dbt->srclistsize + NSELEMSPERCHUNK) * sizeof(ns_src));
		if (new == NULL) {
			return (-1);
		}
		dbt->srclist = new;
	}
	memmove(&dbt->srclist[dbt->srclistsize++], src, sizeof(ns_src));
	return (0);
}


void
_nsdbtdump(dbt)
	const ns_dbt *dbt;
{
	int i;

	_DIAGASSERT(dbt != NULL);

	printf("%s (%d source%s):", dbt->name, dbt->srclistsize,
			dbt->srclistsize == 1 ? "" : "s");
	for (i = 0; i < dbt->srclistsize; i++) {
		printf(" %s", dbt->srclist[i].name);
		if (!(dbt->srclist[i].flags & (NS_UNAVAIL | NS_NOTFOUND | NS_TRYAGAIN))
				&& (dbt->srclist[i].flags & NS_SUCCESS)) {
			continue;
		}
		printf(" [");
		if (!(dbt->srclist[i].flags & NS_SUCCESS)) {
			printf(" SUCCESS=continue");
		}
		if (dbt->srclist[i].flags & NS_UNAVAIL) {
			printf(" UNAVAIL=return");
		}
		if (dbt->srclist[i].flags & NS_NOTFOUND) {
			printf(" NOTFOUND=return");
		}
		if (dbt->srclist[i].flags & NS_TRYAGAIN) {
			printf(" TRYAGAIN=return");
		}
		printf(" ]");
	}
	printf("\n");
}


const ns_dbt *
_nsdbtget(name)
	const char	*name;
{
	static	time_t	 confmod;

	struct stat	 statbuf;
	ns_dbt		 dbt;

	_DIAGASSERT(name != NULL);

	dbt.name = name;

	if (confmod) {
		if (stat(_PATH_NS_CONF, &statbuf) == -1) {
			return (NULL);
		}
		if (confmod < statbuf.st_mtime) {
			int i, j;

			for (i = 0; i < _nsmapsize; i++) {
				for (j = 0; j < _nsmap[i].srclistsize; j++) {
					if (_nsmap[i].srclist[j].name != NULL) {
						/*LINTED const cast*/
						free(/*(void *)*/__UNCONST(_nsmap[i].srclist[j].name));
					}
				}
				if (_nsmap[i].srclist) {
					free(_nsmap[i].srclist);
				}
				if (_nsmap[i].name) {
					/*LINTED const cast*/
					free(/*(void *)*/__UNCONST(_nsmap[i].name));
				}
			}
			if (_nsmap) {
				free(_nsmap);
			}
			_nsmap = NULL;
			_nsmapsize = 0;
			confmod = 0;
		}
	}
	if (!confmod) {
		if (stat(_PATH_NS_CONF, &statbuf) == -1) {
			return (NULL);
		}
		_nsyyin = fopen(_PATH_NS_CONF, "r");
		if (_nsyyin == NULL) {
			return (NULL);
		}
		_nsyyparse();
		(void) fclose(_nsyyin);
		qsort(_nsmap, (size_t) _nsmapsize, sizeof(ns_dbt), _nscmp);
		confmod = statbuf.st_mtime;
	}
	return (bsearch(&dbt, _nsmap, (size_t) _nsmapsize, sizeof(ns_dbt), _nscmp));
}


int
_nsdbtput(dbt)
	const ns_dbt *dbt;
{
	int	i;

	_DIAGASSERT(dbt != NULL);

	for (i = 0; i < _nsmapsize; i++) {
		if (_nscmp(dbt, &_nsmap[i]) == 0) {
			/* overwrite existing entry */
			if (_nsmap[i].srclist != NULL) {
				free(_nsmap[i].srclist);
			}
			memmove(&_nsmap[i], dbt, sizeof(ns_dbt));
			return (0);
		}
	}

	if ((_nsmapsize % NSELEMSPERCHUNK) == 0) {
		ns_dbt *new;

		new = (ns_dbt*) realloc(_nsmap,
				(_nsmapsize + NSELEMSPERCHUNK) * sizeof(ns_dbt));
		if (new == NULL) {
			return (-1);
		}
		_nsmap = new;
	}
	memmove(&_nsmap[_nsmapsize++], dbt, sizeof(ns_dbt));
	return (0);
}


int
/*ARGSUSED*/
nsdispatch(void *retval, const ns_dtab disp_tab[], const char *database, const char *method, const ns_src defaults[], ...)
{
	va_list		 ap;
	int		 i, curdisp, result;
	const ns_dbt	*dbt;
	const ns_src	*srclist;
	int		 srclistsize;

	_DIAGASSERT(database != NULL);
	_DIAGASSERT(method != NULL);

	if (database == NULL || method == NULL) {
		return (NS_UNAVAIL);
	}

	dbt = _nsdbtget(database);
	if (dbt != NULL) {
		srclist = dbt->srclist;
		srclistsize = dbt->srclistsize;
	} else {
		srclist = defaults;
		srclistsize = 0;
		while (srclist[srclistsize].name != NULL) {
			srclistsize++;
		}
	}
	result = 0;

	for (i = 0; i < srclistsize; i++) {
		for (curdisp = 0; disp_tab[curdisp].src != NULL; curdisp++) {
			if (strcasecmp(disp_tab[curdisp].src, srclist[i].name) == 0) {
				break;
			}
		}
		result = 0;
		if (disp_tab[curdisp].callback) {
			va_start(ap, defaults);
			result = disp_tab[curdisp].callback(retval,
					disp_tab[curdisp].cb_data, ap);
			va_end(ap);
			if (result & srclist[i].flags) {
				break;
			}
		}
	}
	return (result ? result : NS_NOTFOUND);
}
