#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "temperament.h"
#include "util.h"

static void usage(void);

static void printnotes(Temperament *t);

int
main(int argc, char *argv[])
{
	FILE *input;
	Temperament t;
	char errbuf[256];

	if (argc != 2)
		usage();

	if (!(input = fopen(argv[1], "r"))) {
		perror("temperatune: cannot open temperament file");
		return 1;
	}
	if (tparse(&t, input, errbuf, sizeof(errbuf))) {
		fprintf(stderr, "temperatune: %s\n", errbuf);
		return 1;
	}

	printf("name: %s\n", t.name);
	if (t.desc)
		printf("description: %s\n", t.desc);
	if (t.src)
		printf("source: %s\n", t.src);
	printf("octave base: %s\n", t.octavebase);
	printf("reference pitch: %.2lf\n", t.refpitch);
	printf("reference note: %s\n", t.refname);
	printf("reference octave: %d\n", t.refoctave);

	printf("notes:\n");
	printnotes(&t);

	tfreefields(&t);
	return 0;
}

static void
usage(void)
{
	fprintf(stderr, "usage: temperatune INPUT\n");
	exit(2);
}

static void
printnotes(Temperament *t)
{
	char **notes;
	size_t nnotes;
	size_t i;
	double offset;

	nnotes = ntabsize(&t->notes);
	notes = malloc(nnotes * sizeof(*notes));
	ntabstorenames(&t->notes, notes);
	ntabsortnames(&t->notes, notes, nnotes);
	for (i = 0; i < nnotes; i++) {
		ntabget(&t->notes, notes[i], &offset);
		printf("%s: %.2lf\n", notes[i], offset);
		free(notes[i]);
	}
	free(notes);
}
