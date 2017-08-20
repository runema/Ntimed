/*-
 * Copyright (c) 2014 Poul-Henning Kamp
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Operational Context STDIO
 * =========================
 *
 *	"The most effective debugging tool is still careful thought,
 *	 coupled with judiciously placed print statements."
 *		-- Brian Kernighan, "Unix for Beginners" (1979)
 *
 * The problem with print statements is where they end up.  For instance
 * in a server with a CLI interface, you want the print statements to go
 * to the CLI session which called the code.
 *
 * An "Operational Context" is a back-pointer to where the print statement
 * should end up.
 *
 * We operate with three "channels", DIAG, TRACE and DEBUG.
 *
 * DIAG is mandatory output, error messages, diagnostics etc.
 *	This should always end up where the action was initiated.
 *
 * DEBUG is optional output which may be supressed.
 *	This should go where DIAG goes, unless specifically redirected
 *	by the operator.
 *
 * TRACE is data collection, statistics etc.
 *	In general this goes nowhere unless configured to end up
 *	somewhere.
 *
 * About this implementation:
 *
 * This is a very naive implementation spitting things out to stdout/stderr,
 * knowing that we are a single threaded program.
 *
 * XXX: Pull in sbufs to do it right.
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "ntimed.h"

static FILE *tracefile;

static FILE *
getdst(enum ocx_chan chan)
{
	if (chan == OCX_DIAG)
		return (stderr);
	if (chan == OCX_TRACE)
		return (tracefile);
	if (chan == OCX_DEBUG)
		return (stdout);
	WRONG("Wrong ocx_chan");
	NEEDLESS_RETURN(NULL);
}

void __match_proto__()
putv(struct ocx *ocx, enum ocx_chan chan, const char *fmt, va_list ap)
{
	FILE *dst = getdst(chan);
	va_list ap2;

	va_copy(ap2, ap);
	AZ(ocx);
	if (dst != NULL)
		(void)vfprintf(dst, fmt, ap);
	if (chan == OCX_DIAG)
		vsyslog(LOG_ERR, fmt, ap2);
	va_end(ap2);
}

/**********************************************************************
 * XXX: take strftime format string to chop tracefiles in time.
 */

void
ArgTracefile(struct ocx *ocx, const char *fn)
{

	if (tracefile != NULL && tracefile != stdout) {
		AZ(fclose(tracefile));
		tracefile = NULL;
	}

	if (fn == NULL)
		return;

	if (!strcmp(fn, "-")) {
		tracefile = stdout;
		return;
	}

	tracefile = fopen(fn, "w");
	if (tracefile == NULL)
		Fail(ocx, 1, "Could not open '%s' for writing", fn);
	setbuf(tracefile, NULL);
}

