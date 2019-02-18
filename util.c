/*
 * Copyright (c) 2019 Ian Johnson
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void
die(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	fprintf(stderr, "temperatune: ");
	vfprintf(stderr, fmt, args);
	if (errno != 0)
		fprintf(stderr, ": %s", strerror(errno));
	fprintf(stderr, "\n");
	va_end(args);
	exit(1);
}

void *
xmalloc(size_t sz)
{
	void *ret;

	if (!(ret = malloc(sz)))
		die("xmalloc: out of memory");
	return ret;
}

void *xcalloc(size_t n, size_t sz)
{
	void *ret;

	if (!(ret = calloc(n, sz)))
		die("xcalloc: out of memory");
	return ret;
}

char *xstrdup(const char *s)
{
	char *ret;

	if (!(ret = strdup(s)))
		die("xstrdup: out of memory");
	return ret;
}
