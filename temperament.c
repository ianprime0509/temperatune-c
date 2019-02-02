#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "temperament.h"
#include "util.h"

enum { CENTS_IN_OCTAVE = 1200 };

enum { TABLE_SIZE = 17 };

struct notes {
	struct note *notes[TABLE_SIZE];
};

struct note {
	char *name;
	/** The offset, in cents, from the base note. */
	float offset;
	struct note *next;
};

static unsigned int hash(const char *str);

float
temperament_get_pitch(const struct temperament *t, const char *note, int octave)
{
	float offset;

	if (t == NULL)
		return -1;

	if (notes_get_offset(t->notes, note, &offset))
		return -1;
	offset += (octave - t->reference_octave) * CENTS_IN_OCTAVE;
	return t->reference_pitch * powf(2, offset / CENTS_IN_OCTAVE);
}

int
notes_add(struct notes *notes, const char *name, float offset)
{
	unsigned int bucket;
	struct note *note;

	if (notes == NULL)
		return 1;

	bucket = hash(name) % TABLE_SIZE;
	for (note = notes->notes[bucket]; note; note = note->next)
		if (!strcmp(name, note->name)) {
			note->offset = offset;
			return 0;
		}

	note = xmalloc(sizeof(*note));
	note->name = xstrdup(name);
	note->offset = offset;
	note->next = notes->notes[bucket];
	notes->notes[bucket] = note;
	return 0;
}

struct notes *
notes_alloc(void)
{
	return xcalloc(1, sizeof(struct notes));
}

void
notes_free(struct notes *notes)
{
	struct note *note, *next;

	if (!notes)
		return;

	for (int i = 0; i < TABLE_SIZE; i++) {
		note = notes->notes[i];
		while (note) {
			next = note->next;
			free(note->name);
			free(note);
			note = next;
		}
	}
	free(notes);
}

char **
notes_get_names(const struct notes *notes)
{
	size_t size, n;
	char **names;
	int i;
	const struct note *note;

	size = notes_size(notes);
	names = xmalloc((size + 1) * sizeof(*names));
	names[size] = NULL;

	n = 0;
	for (i = 0; i < TABLE_SIZE; i++)
		for (note = notes->notes[i]; note; note = note->next)
			names[n++] = xstrdup(note->name);
	return names;
}

int
notes_get_offset(const struct notes *notes, const char *name, float *offset)
{
	unsigned int bucket;
	const struct note *note;

	if (!notes)
		return 1;

	bucket = hash(name) % TABLE_SIZE;
	for (note = notes->notes[bucket]; note; note = note->next)
		if (!strcmp(name, note->name)) {
			if (offset)
				*offset = note->offset;
			return 0;
		}
	return 1;
}

size_t
notes_size(const struct notes *notes)
{
	size_t size;
	int i;
	const struct note *note;

	size = 0;
	for (i = 0; i < TABLE_SIZE; i++)
		for (note = notes->notes[i]; note; note = note->next)
			size++;
	return size;
}

static unsigned int
hash(const char *str)
{
	unsigned int hash;

	if (!str)
		return 0;

	hash = 0;
	while (*str)
		hash += 31 * *str++;
	return hash;
}
