%{
/*	$NetBSD: nslexer.l,v 1.7 2004/01/25 16:38:15 lukem Exp $	*/

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
__RCSID("$NetBSD: nslexer.l,v 1.7 2004/01/25 16:38:15 lukem Exp $");
#endif /* LIBC_SCCS and not lint */

#include "namespace.h"
#include <ctype.h>
#define _NS_PRIVATE
#include <nsswitch.h>
#include <string.h>
#include <syslog.h>

#include "nsparser.h"

%}

%option yylineno nounput noinput
%option never-interactive

BLANK		[ \t]
CR		\n
STRING		[a-zA-Z][a-zA-Z0-9_]*

%%

{BLANK}+	;			/* skip whitespace */

#.*		;			/* skip comments */

\\{CR}		;			/* allow continuation */

{CR}		return NL;

[sS][uU][cC][cC][eE][sS][sS]		return SUCCESS;
[uU][nN][aA][vV][aA][iI][lL]		return UNAVAIL;
[nN][oO][tT][fF][oO][uU][nN][dD]	return NOTFOUND;
[tT][rR][yY][aA][gG][aA][iI][nN]	return TRYAGAIN;

[rR][eE][tT][uU][rR][nN]		return RETURN;
[cC][oO][nN][tT][iI][nN][uU][eE]	return CONTINUE;

{STRING}	{
			char *p;
			size_t i;

			if ((p = strdup(yytext)) == NULL) {
				syslog(LOG_ERR, "libc nsdispatch: ");
				return NL;
			}

			for (i = 0; i < strlen(p); i++) {
				if (isupper((unsigned char)p[i]))
					p[i] = tolower((unsigned char)p[i]);
			}
			_nsyylval.str = p;
			return STRING;
		}

.		return yytext[0];

%%

#undef _nsyywrap
int
_nsyywrap()
{
	return 1;
} /* _nsyywrap */

void
_nsyyerror(msg)
	const char *msg;
{

	 syslog(LOG_WARNING, "libc nsdispatch: %s line %d: %s at '%s'",
	    _PATH_NS_CONF, yylineno, msg, yytext);
} /* _nsyyerror */
