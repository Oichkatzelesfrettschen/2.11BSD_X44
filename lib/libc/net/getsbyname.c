/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <sys/cdefs.h>
#if defined(LIBC_SCCS) && !defined(lint)
#if 0
static char sccsid[] = "@(#)getservbyname.c	5.3 (Berkeley) 5/19/86";
static char sccsid[] = "@(#)getservbyname.c	8.1 (Berkeley) 6/4/93";
#endif
#endif /* LIBC_SCCS and not lint */

#include "namespace.h"

#include <netdb.h>
#include <string.h>

#include "servent.h"

#ifdef __weak_alias
__weak_alias(getservbyname_r,_getservbyname_r)
__weak_alias(getservbyname,_getservbyname)
#endif

int
getservbyname_r(sp, sd, name, proto, buffer, buflen, result)
	struct servent *sp;
	struct servent_data *sd;
	const char *name, *proto;
	char *buffer;
	size_t buflen;
	struct servent **result;
{
	register char **cp;
	int rval;

	setservent_r(sd->stayopen, sd);
	while ((rval = getservent_r(sp, sd, buffer, buflen, result))) {
		if (strcmp(name, sp->s_name) == 0) {
			goto gotname;
		}
		for (cp = sp->s_aliases; *cp; cp++) {
			if (strcmp(name, *cp) == 0) {
				goto gotname;
			}
		}
		continue;
gotname:
		if (proto == 0 || strcmp(sp->s_proto, proto) == 0) {
			break;
		}
	}
	if (!sd->stayopen) {
		endservent_r(sd);
	}
	return (rval);
}

struct servent *
getservbyname(name, proto)
	const char *name, *proto;
{
	struct servent *p;
	int rval;

	rval = getservbyname_r(&_svs_serv, &_svs_servd, name, proto, _svs_servbuf, sizeof(_svs_servbuf), &p);
	return ((rval == 1) ? p : NULL);
}

#ifdef original

extern int _serv_stayopen;

struct servent *
getservbyname(name, proto)
	const char *name, *proto;
{
	register struct servent *p;
	register char **cp;

	setservent(_serv_stayopen);
	while ((p = getservent())) {
		if (strcmp(name, p->s_name) == 0)
			goto gotname;
		for (cp = p->s_aliases; *cp; cp++)
			if (strcmp(name, *cp) == 0)
				goto gotname;
		continue;
gotname:
		if (proto == 0 || strcmp(p->s_proto, proto) == 0)
			break;
	}
	if (!_serv_stayopen)
		endservent();
	return (p);
}
#endif
