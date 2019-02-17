typedef struct Sinebuf Sinebuf;

struct Sinebuf {
	float *samp; /* precomputed mono sample buffer */
	size_t pos; /* current position in the buffer */
	size_t nsamp; /* number of samples */
};

int sbcallback(const void *input, void *output, unsigned long framecnt, const PaStreamCallbackTimeInfo *tminfo, PaStreamCallbackFlags statflags, void *sb);
void sbfill(Sinebuf *sb, float *buf, size_t nframes);
int sbinit(Sinebuf *sb, double freq, double samprate, double volume);
