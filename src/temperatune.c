#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <portaudio.h>

#include "audio.h"
#include "util.h"

const double SAMPRATE = 44100;

static void usage(void);

int
main(int argc, char *argv[])
{
	Sinebuf sb;
	PaStream *stream;
	PaError err;

	if (argc != 2)
		usage();

	sbinit(&sb, atof(argv[1]), SAMPRATE);
	err = Pa_Initialize();
	if (err != paNoError)
		die("could not initialize PortAudio: %s", Pa_GetErrorText(err));

	err = Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, SAMPRATE, paFramesPerBufferUnspecified, sbcallback, &sb);
	if (err != paNoError)
		die("could not open stream: %s", Pa_GetErrorText(err));

	err = Pa_StartStream(stream);
	if (err != paNoError)
		die("could not start stream: %s", Pa_GetErrorText(err));
	Pa_Sleep(2000);
	err = Pa_AbortStream(stream);
	if (err != paNoError)
		die("could not abort stream: %s", Pa_GetErrorText(err));
	err = Pa_Terminate();
	if (err != paNoError)
		die("could not terminate PortAudio: %s", Pa_GetErrorText(err));

	return 0;
}

static void
usage(void)
{
	fprintf(stderr, "usage: temperatune FREQ\n");
	exit(2);
}
