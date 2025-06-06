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
static char sccsid[] = "@(#)delete.c	5.6 (Berkeley) 3/12/91";
#endif

#include <sys/dir.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "archive.h"
#include "extern.h"
#include "pathnames.h"
#include <ar.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern CHDR chdr;	  /* converted header */
extern char *archive; /* archive name */
extern char *tname;	  /* temporary file "name" */
extern u_int options;

/*-
 * delete --
 *	Deletes named members from the archive.
 */
int delete(char **argv) {
	CF cf;
	off_t size;
	int afd, tfd;
	char *file;

	afd = open_archive(O_RDWR);
	tfd = tmp();

	/* Read and write to an archive; pad on both. */
	SETCF(afd, archive, tfd, tname, RPAD | WPAD);
	while (get_arobj(afd)) {
		if (*argv && (file = files(argv))) {
			if (options & AR_V)
				(void)printf("d - %s\n", file);
			skip_arobj(afd);
			continue;
		}
		put_arobj(&cf, (struct stat *)NULL);
	}

	size = lseek(tfd, (off_t)0, L_INCR);
	(void)lseek(tfd, (off_t)0, L_SET);
	(void)lseek(afd, (off_t)SARMAG, L_SET);
	SETCF(tfd, tname, afd, archive, NOPAD);
	copy_ar(&cf, size);
	(void)close(tfd);
	//	(void)ftruncate(afd, size + SARMAG);
	//  close_archive(afd);
	if (ftruncate(afd, size + SARMAG)) {
		close_archive(afd);
	}

	if (*argv) {
		orphans(argv);
		return (1);
	}
	return (0);
}
