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
