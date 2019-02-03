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
	char **notes, **noteptr;
	double offset;

	notes = notes_get_names(t->notes);
	for (noteptr = notes; *noteptr; noteptr++) {
		notes_get_offset(t->notes, *noteptr, &offset);
		printf("'%s': %lf\n", *noteptr, offset);
		free(*noteptr);
	}
	free(notes);
}
