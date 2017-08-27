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
 * Theis contains the generic stuff. Se ocx_stdio.c for the details.
 *
 */

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "ntimed.h"

void
Put(struct ocx *ocx, enum ocx_chan chan, const char *fmt, ...)
{
	va_list ap;

	AZ(ocx);
	va_start(ap, fmt);
	putv(ocx, chan, fmt, ap);
	va_end(ap);
}

void
PutHex(struct ocx *ocx, enum ocx_chan chan, const void *ptr, ssize_t len)
{
	const uint8_t *p = ptr;
	const char *s = "";

	AN(ptr);
	assert(len >= 0);

	while(len--) {
		Put(ocx, chan, "%s%02x", s, *p++);
		s = " ";
	}
}

void
Fail(struct ocx *ocx, int err, const char *fmt, ...)
{
	va_list ap;

	if (err)
		err = errno;
	Put(ocx, OCX_DIAG, "Failure: ");
	va_start(ap, fmt);
	putv(ocx, OCX_DIAG, fmt, ap);
	va_end(ap);
	Put(ocx, OCX_DIAG, "\n");
	if (err)
		Put(ocx, OCX_DIAG, "errno = %d (%s)\n", err, strerror(err));
	exit(1);
}
