/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Hugh Smith at The University of Guelph.
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

#if HAVE_NBTOOL_CONFIG_H
#include "nbtool_config.h"
#endif

#include <sys/cdefs.h>
#if defined(DOSCCS) && !defined(lint)
static char sccsid[] = "@(#)contents.c	5.6 (Berkeley) 3/12/91";
#endif

#include "archive.h"
#include "extern.h"
#include <ar.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

extern CHDR chdr;	  /* converted header */
extern char *archive; /* archive name */
extern u_int options;

/*
 * contents --
 *	Handles t[v] option - opens the archive and then reads headers,
 *	skipping member contents.
 */
int contents(char **argv) {
	int afd, all;
	struct tm *tp;
	char *file, buf[25];

	afd = open_archive(O_RDONLY);

	for (all = !*argv; get_arobj(afd);) {
		if (all)
			file = chdr.name;
		else if (!(file = files(argv)))
			goto next;
		if (options & AR_V) {
			(void)strmode(chdr.mode, buf);
			(void)printf("%s %6d/%-6d %8ld ", buf + 1, chdr.uid, chdr.gid,
						 chdr.size);
			tp = localtime(&chdr.date);
#ifdef bloat
			(void)strftime(buf, sizeof(buf), "%b %e %H:%M %Y", tp);
			(void)printf("%s %s\n", buf, file);
#else
			{
				/*
				 * Bloat avoidance alert!  Doing the date this way saves
				 * dragging in not only strftime() but mktime() and ctime() for
				 * a savings of over 8kb.  God, those leap seconds or whatever
				 * don't come cheap, the old (4.3BSD) timezone code was big
				 * enough but this new stuff (posix?) is horrid.
				 */
				static char *months[] = {"Jan", "Feb", "Mar", "Apr",
										 "May", "Jun", "Jul", "Aug",
										 "Sep", "Oct", "Nov", "Dec"};

				(void)printf("%s %02d %02d:%02d %4d %s\n", months[tp->tm_mon],
							 tp->tm_mday, tp->tm_hour, tp->tm_min,
							 1900 + tp->tm_year, file);
			}
#endif
		} else
			(void)printf("%s\n", file);
		if (!all && !*argv)
			break;
	next:
		skip_arobj(afd);
	}
	close_archive(afd);

	if (*argv) {
		orphans(argv);
		return (1);
	}
	return (0);
}
