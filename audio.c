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

#include <math.h>
#include <stdlib.h>

#include <portaudio.h>

#include "audio.h"
#include "util.h"

const float PI = 3.14159f;
const double MINFREQ = 25, MAXFREQ = 8000;

int
sbcallback(const void *input, void *output, unsigned long framecnt, const PaStreamCallbackTimeInfo *tminfo, PaStreamCallbackFlags statflags, void *sb)
{
	USED(input);
	USED(tminfo);
	USED(statflags);
	sbfill(sb, output, framecnt);
	return 0;
}

void
sbfill(Sinebuf *sb, float *buf, size_t nframes)
{
	while (nframes-- > 0) {
		*buf++ = sb->samp[sb->pos++];
		if (sb->pos >= sb->nsamp)
			sb->pos = 0;
	}
}

int
sbinit(Sinebuf *sb, double freq, double samprate, double volume)
{
	double period;
	size_t i;

	if (freq < MINFREQ || freq > MAXFREQ || volume < 0 || volume > 1 || samprate <= 0)
		return 1;

	period = samprate / freq;
	sb->nsamp = (size_t)round(period);
	sb->samp = xmalloc(sb->nsamp * sizeof(*sb->samp));
	for (i = 0; i < sb->nsamp; i++)
		sb->samp[i] = volume * sinf(2 * PI * i / period);
	sb->pos = 0;
	return 0;
}
