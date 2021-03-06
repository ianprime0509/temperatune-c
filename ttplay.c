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
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <portaudio.h>

#include "audio.h"
#include "temperament.h"
#include "util.h"

#define SAMPRATE 44100

static void
usage(void)
{
	fprintf(stderr, "usage: temperatune [-r reference] [-t time] [-v volume] temperament note octave\n");
	exit(2);
}

static void
play(double freq, double volume, unsigned int time)
{
	Sinebuf sb;
	PaStream *stream;
	const char *errmsg;
	PaError err;

	if (sbinit(&sb, freq, SAMPRATE, volume))
		die("pitch out of range: %lf Hz", freq);
	if ((err = Pa_Initialize()) != paNoError) {
		errmsg = "could not initialize PortAudio: %s";
		goto FAIL;
	}

	err = Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, SAMPRATE, paFramesPerBufferUnspecified, sbcallback, &sb);
	if (err != paNoError) {
		errmsg = "could not open stream: %s";
		goto FAIL;
	}

	if ((err = Pa_StartStream(stream)) != paNoError) {
		errmsg = "could not start stream: %s";
		goto FAIL;
	}
	sleep(time);
	if ((err = Pa_AbortStream(stream)) != paNoError) {
		errmsg = "could not abort stream: %s";
		goto FAIL;
	}

	Pa_Terminate();
	return;

FAIL:
	Pa_Terminate();
	die(errmsg, Pa_GetErrorText(err));
}

int
main(int argc, char *argv[])
{
	int opt;
	unsigned long time;
	double volume, freq, refpitch;
	char *end, errbuf[256];
	FILE *tfile;
	Temperament t;
	long octave;

	time = 5;
	volume = 0.5;
	refpitch = 0;
	while ((opt = getopt(argc, argv, ":r:t:v:")) != -1)
		switch (opt) {
		case 'r':
			errno = 0;
			refpitch = strtod(optarg, &end);
			if (errno != 0 || *end != '\0' || *optarg == '\0' || refpitch <= 0)
				die("bad reference pitch: '%s'", optarg);
			break;
		case 't':
			errno = 0;
			time = strtoul(optarg, &end, 10);
			if (errno != 0 || *end != '\0' || *optarg == '\0' || time > UINT_MAX)
				die("bad time: '%s'", optarg);
			break;
		case 'v':
			errno = 0;
			volume = strtod(optarg, &end);
			if (errno != 0 || *end != '\0' || *optarg == '\0' || volume < 0 || volume > 1)
				die("bad volume: '%s'", optarg);
			break;
		case ':':
			fprintf(stderr, "'%c' expects an argument", optopt);
			usage();
			/* Break here to silence a warning, even though it's not necessary. */
			break;
		case '?':
			fprintf(stderr, "unknown option '%c'", optopt);
			usage();
			break;
		}

	if (optind != argc - 3)
		usage();

	octave = strtol(argv[optind + 2], &end, 10);
	if (errno != 0 || *end != '\0' || *argv[optind + 2] == '\0' || octave < INT_MIN || octave > INT_MAX)
		die("bad octave: '%s'", argv[optind + 2]);

	if (!(tfile = fopen(argv[optind], "r")))
		die("could not open temperament file");
	if (tparse(&t, tfile, errbuf, sizeof(errbuf)))
		die("%s", errbuf);
	if (refpitch > 0)
		t.refpitch = refpitch;
	if ((freq = tgetpitch(&t, argv[optind + 1], octave)) < 0)
		die("bad note: '%s'", argv[optind + 1]);

	play(freq, volume, time);

	return 0;
}

