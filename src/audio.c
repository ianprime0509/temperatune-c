#include <math.h>
#include <stdlib.h>

#include <portaudio.h>

#include "audio.h"
#include "util.h"

const float PI = 3.14159f;

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

void
sbinit(Sinebuf *sb, double freq, double samprate)
{
	double period;
	size_t i;

	period = samprate / freq;
	sb->nsamp = (size_t)round(period);
	sb->samp = xmalloc(sb->nsamp * sizeof(*sb->samp));
	for (i = 0; i < sb->nsamp; i++)
		sb->samp[i] = sinf(2 * PI * i / period);
	sb->pos = 0;
}
