#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "temperament.h"
#include "util.h"

enum { TABLE_SIZE = 17 };

struct notes {
	struct note *notes[TABLE_SIZE];
};

struct note {
	char *name;
	/** The offset, in cents, from the base note. */
	double offset;
	struct note *next;
};

static unsigned int hash(const char *str);

void
temperament_free_contents(struct temperament *t)
{
	free(t->name);
	free(t->description);
	free(t->source);
	free(t->octave_base_name);
	free(t->reference_name);
	notes_free(t->notes);
}

double
temperament_get_pitch(const struct temperament *t, const char *note, int octave)
{
	double offset;

	if (t == NULL)
		return -1;

	if (notes_get_offset(t->notes, note, &offset))
		return -1;
	offset += (octave - t->reference_octave) * CENTS_IN_OCTAVE;
	return t->reference_pitch * powf(2, offset / CENTS_IN_OCTAVE);
}

void
temperament_normalize(struct temperament *t)
{
	double baseoffset, reloffset;
	struct note *note;
	int i;

	/* The octave base must be below (or at) the reference pitch. */
	notes_get_offset(t->notes, t->octave_base_name, &baseoffset);
	baseoffset = fmod(baseoffset, CENTS_IN_OCTAVE);
	if (baseoffset > 0)
		baseoffset -= CENTS_IN_OCTAVE;
	notes_add(t->notes, t->octave_base_name, baseoffset);

	/*
	 * Make sure all other offsets are above the octave base and within
	 * an octave.
	 */
	for (i = 0; i < TABLE_SIZE; i++)
		for (note = t->notes->notes[i]; note; note = note->next) {
			reloffset = note->offset - baseoffset;
			reloffset = fmod(reloffset, CENTS_IN_OCTAVE);
			if (reloffset < 0)
				reloffset += CENTS_IN_OCTAVE;
			note->offset = baseoffset + reloffset;
		}
}

int
notes_add(struct notes *notes, const char *name, double offset)
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

int
notes_get_offset(const struct notes *notes, const char *name, double *offset)
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

void
notes_sort_names(const struct notes *notes, char *names[], size_t nnames)
{
	size_t i, j;
	double off1, off2;
	char *tmp;

	/*
	 * qsort would require a global variable, so for now here's
	 * insertion sort.
	 */
	for (i = 0; i < nnames; i++)
		for (j = i; j > 0; j--) {
			notes_get_offset(notes, names[j - 1], &off1);
			notes_get_offset(notes, names[j], &off2);
			if (off1 > off2) {
				tmp = names[j - 1];
				names[j - 1] = names[j];
				names[j] = tmp;
			} else {
				break;
			}
		}
}

void
notes_store_names(const struct notes *notes, char *names[])
{
	int i;
	const struct note *note;

	for (i = 0; i < TABLE_SIZE; i++)
		for (note = notes->notes[i]; note; note = note->next)
			*names++ = xstrdup(note->name);
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
