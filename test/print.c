#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "temperament.h"
#include "util.h"

static void usage(void);

static void print_notes(const struct temperament *t);

int
main(int argc, char *argv[])
{
	FILE *input;
	struct temperament t;
	char errbuf[256];

	if (argc != 2)
		usage();

	if (!(input = fopen(argv[1], "r"))) {
		perror("temperatune: cannot open temperament file");
		return 1;
	}
	if (temperament_parse(&t, input, errbuf, sizeof(errbuf))) {
		fprintf(stderr, "temperatune: %s\n", errbuf);
		return 1;
	}

	printf("name: %s\n", t.name);
	if (t.description)
		printf("description: %s\n", t.description);
	if (t.source)
		printf("source: %s\n", t.source);
	printf("octave base: %s\n", t.octave_base_name);
	printf("reference pitch: %.2lf\n", t.reference_pitch);
	printf("reference note: %s\n", t.reference_name);
	printf("reference octave: %d\n", t.reference_octave);

	printf("notes:\n");
	print_notes(&t);

	return 0;
}

static void
usage(void)
{
	fprintf(stderr, "usage: temperatune INPUT\n");
	exit(2);
}

static void
print_notes(const struct temperament *t)
{
	char **notes;
	size_t nnotes;
	size_t i;
	double offset;

	nnotes = notes_size(t->notes);
	notes = malloc(nnotes * sizeof(*notes));
	notes_store_names(t->notes, notes);
	notes_sort_names(t->notes, notes, nnotes);
	for (i = 0; i < nnotes; i++) {
		notes_get_offset(t->notes, notes[i], &offset);
		printf("%s: %.2lf\n", notes[i], offset);
		free(notes[i]);
	}
	free(notes);
}
