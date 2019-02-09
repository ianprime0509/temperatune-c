#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <jansson.h>

#include "temperament.h"
#include "util.h"

typedef struct Notestack Notestack;

struct Notestack {
	const char *name;
	Notestack *next;
};

static Notestack *nspush(Notestack *ns, const char *name);
static Notestack *nspop(Notestack *ns, const char **name);

static void error(char *errbuf, size_t errsize, char *fmt, ...);

static int assignoffset(Notetab *ntab, const char *name, double offset, char *errbuf, size_t errsize);
static int processnote(Notestack **todo, json_t *notedefs, Notetab *ntab, char *errbuf, size_t errsize);
static int tpopulate(Temperament *t, json_t *root, char *errbuf, size_t errsize);
static int tpopulatenotes(Temperament *t, json_t *notedefs, char *errbuf, size_t errsize);
static int validatenotes(json_t *notedefs, char *errbuf, size_t errsize);

static unsigned int hash(const char *str);

void
tfreefields(Temperament *t)
{
	free(t->name);
	free(t->desc);
	free(t->src);
	free(t->octavebase);
	free(t->refname);
	ntabfreenotes(&t->notes);
}

double
tgetpitch(Temperament *t, const char *note, int octave)
{
	double offset;

	if (ntabget(&t->notes, note, &offset))
		return -1;
	offset += (octave - t->refoctave) * OCTAVE_CENTS;
	return t->refpitch * powf(2, offset / OCTAVE_CENTS);
}

void
tnormalize(Temperament *t)
{
	double baseoffset, reloffset;
	Note *note;
	int i;

	/* The octave base must be below (or at) the reference pitch. */
	ntabget(&t->notes, t->octavebase, &baseoffset);
	baseoffset = fmod(baseoffset, OCTAVE_CENTS);
	if (baseoffset > 0)
		baseoffset -= OCTAVE_CENTS;
	ntabadd(&t->notes, t->octavebase, baseoffset);

	/*
	 * Make sure all other offsets are above the octave base and within
	 * an octave.
	 */
	for (i = 0; i < TABSIZE; i++)
		for (note = t->notes[i]; note; note = note->next) {
			reloffset = note->offset - baseoffset;
			reloffset = fmod(reloffset, OCTAVE_CENTS);
			if (reloffset < 0)
				reloffset += OCTAVE_CENTS;
			note->offset = baseoffset + reloffset;
		}
}

int
tparse(Temperament *t, FILE *input, char *errbuf, size_t errsize)
{
	json_t *root;
	json_error_t err;
	Temperament tmp;
	int retval;

	retval = 0;
	root = json_loadf(input, 0, &err);
	if (!root) {
		error(errbuf, errsize, "could not parse input: %s", err.text);
		retval = 1;
		goto EXIT;
	}

	retval = tpopulate(&tmp, root, errbuf, errsize);
	if (retval)
		goto EXIT;

	*t = tmp;
	tnormalize(t);

EXIT:
	json_decref(root);
	return retval;
}

void
ntabadd(Notetab *ntab, const char *name, double offset)
{
	unsigned int bucket;
	Note *note;

	bucket = hash(name) % TABSIZE;
	for (note = (*ntab)[bucket]; note; note = note->next)
		if (!strcmp(name, note->name)) {
			note->offset = offset;
			return;
		}

	note = xmalloc(sizeof(*note));
	note->name = xstrdup(name);
	note->offset = offset;
	note->next = (*ntab)[bucket];
	(*ntab)[bucket] = note;
	return;
}

void
ntabfreenotes(Notetab *ntab)
{
	Note *note, *next;

	for (int i = 0; i < TABSIZE; i++) {
		note = (*ntab)[i];
		while (note) {
			next = note->next;
			free(note->name);
			free(note);
			note = next;
		}
	}
}

int
ntabget(Notetab *ntab, const char *name, double *offset)
{
	unsigned int bucket;
	Note *note;

	bucket = hash(name) % TABSIZE;
	for (note = (*ntab)[bucket]; note; note = note->next)
		if (!strcmp(name, note->name)) {
			if (offset)
				*offset = note->offset;
			return 0;
		}
	return 1;
}

size_t
ntabsize(Notetab *ntab)
{
	size_t size;
	int i;
	Note *note;

	size = 0;
	for (i = 0; i < TABSIZE; i++)
		for (note = (*ntab)[i]; note; note = note->next)
			size++;
	return size;
}

void
ntabsortnames(Notetab *ntab, char *names[], size_t nnames)
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
			ntabget(ntab, names[j - 1], &off1);
			ntabget(ntab, names[j], &off2);
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
ntabstorenames(Notetab *ntab, char *names[])
{
	int i;
	Note *note;

	for (i = 0; i < TABSIZE; i++)
		for (note = (*ntab)[i]; note; note = note->next)
			*names++ = xstrdup(note->name);
}

static Notestack *
nspush(Notestack *ns, const char *name)
{
	Notestack *new;

	new = malloc(sizeof(*new));
	new->name = name;
	new->next = ns;
	return new;
}

static Notestack *
nspop(Notestack *ns, const char **name)
{
	Notestack *next;

	next = ns->next;
	*name = ns->name;
	free(ns);
	return next;
}

static void
error(char *errbuf, size_t errsize, char *fmt, ...)
{
	va_list args;

	if (!errbuf)
		return;
	va_start(args, fmt);
	vsnprintf(errbuf, errsize, fmt, args);
	va_end(args);
}

static int
assignoffset(Notetab *ntab, const char *name, double offset, char *errbuf, size_t errsize)
{
	double prevoffset;

	if (!ntabget(ntab, name, &prevoffset) && fmod(offset - prevoffset, OCTAVE_CENTS) != 0) {
		error(errbuf, errsize, "found conflicting offset for '%s'", name);
		return 1;
	}
	ntabadd(ntab, name, offset);
	return 0;
}

static int
processnote(Notestack **todo, json_t *notedefs, Notetab *ntab, char *errbuf, size_t errsize)
{
	const char *currnote, *newnote, *tmp;
	double curroffset, newoffset;
	json_t *pair;

	*todo = nspop(*todo, &currnote);

	ntabget(ntab, currnote, &curroffset);

	/* Check for the note on the left hand side. */
	pair = json_object_get(notedefs, currnote);
	if (pair) {
		newnote = json_string_value(json_array_get(pair, 0));
		newoffset = json_number_value(json_array_get(pair, 1));

		/*
		Make sure not to add the new note as a "todo" if it's already
		defined. Even if it's already defined, though, we check the
		offset below to detect invalid input (multiple possible values
		for an offset).
		 */
		if (ntabget(ntab, newnote, NULL))
			*todo = nspush(*todo, newnote);

		if (assignoffset(ntab, newnote, curroffset - newoffset, errbuf, errsize))
			return 1;
	}

	/* Check for the note on the right hand side. */
	json_object_foreach(notedefs, newnote, pair) {
		tmp = json_string_value(json_array_get(pair, 0));
		if (!strcmp(tmp, currnote)) {
			newoffset = json_number_value(json_array_get(pair, 1));
			if (ntabget(ntab, newnote, NULL))
				*todo = nspush(*todo, newnote);

			if (assignoffset(ntab, newnote, curroffset + newoffset, errbuf, errsize))
				return 1;
		}
	}

	return 0;
}

static int
tpopulate(Temperament *t, json_t *root, char *errbuf, size_t errsize)
{
	json_t *tmp;
	const char *str;
	double d;

	memset(t, 0, sizeof(*t));

	if (!json_is_object(root)) {
		error(errbuf, errsize, "input is not a JSON object");
		goto FAIL;
	}

	if (!(str = json_string_value(json_object_get(root, "name")))) {
		error(errbuf, errsize, "name not found");
		goto FAIL;
	}
	t->name = xstrdup(str);

	if ((str = json_string_value(json_object_get(root, "description"))))
		t->desc = xstrdup(str);
	else
		t->desc = NULL;

	if ((str = json_string_value(json_object_get(root, "source"))))
		t->src = xstrdup(str);
	else
		t->src = NULL;

	if (!(str = json_string_value(json_object_get(root, "octaveBaseName")))) {
		error(errbuf, errsize, "octave base name not found");
		goto FAIL;
	}
	t->octavebase = xstrdup(str);

	tmp = json_object_get(root, "referencePitch");
	if (!json_is_number(tmp)) {
		error(errbuf, errsize, "reference pitch not found");
		goto FAIL;
	}
	d = json_number_value(tmp);
	if (d <= 0) {
		error(errbuf, errsize, "reference pitch must be greater than zero");
		goto FAIL;
	}
	t->refpitch = d;

	if (!(str = json_string_value(json_object_get(root, "referenceName")))) {
		error(errbuf, errsize, "reference note name not found");
		goto FAIL;
	}
	t->refname = xstrdup(str);

	tmp = json_object_get(root, "referenceOctave");
	if (!json_is_integer(tmp)) {
		error(errbuf, errsize, "reference octave not found");
		goto FAIL;
	}
	t->refoctave = json_integer_value(tmp);

	if (!(tmp = json_object_get(root, "notes"))) {
		error(errbuf, errsize, "notes not found");
		goto FAIL;
	}
	if (validatenotes(tmp, errbuf, errsize))
		goto FAIL;

	if (tpopulatenotes(t, tmp, errbuf, errsize))
		goto FAIL;
	return 0;

FAIL:
	tfreefields(t);
	return 1;
}

static int
tpopulatenotes(Temperament *t, json_t *notedefs, char *errbuf, size_t errsize)
{
	Notetab ntab;
	Notestack *todo;
	const char *note;
	json_t *pair;

	memset(&ntab, 0, sizeof(ntab));
	ntabadd(&ntab, t->refname, 0);
	todo = nspush(NULL, t->refname);

	while (todo)
		if (processnote(&todo, notedefs, &ntab, errbuf, errsize))
			goto FAIL;

	/* Ensure we have all the notes we need and none are undefined. */
	if (ntabget(&ntab, t->octavebase, NULL)) {
		error(errbuf, errsize, "could not determine offset of octave base '%s'", t->octavebase);
		goto FAIL;
	}

	json_object_foreach(notedefs, note, pair) {
		if (ntabget(&ntab, note, NULL)) {
			error(errbuf, errsize, "no offset determined for note '%s'", note);
			goto FAIL;
		}
	}

	memcpy(&t->notes, &ntab, sizeof(ntab));
	return 0;

FAIL:
	ntabfreenotes(&ntab);
	return 1;
}

static int
validatenotes(json_t *notedefs, char *errbuf, size_t errsize)
{
	const char *note;
	json_t *pair;

	if (!json_is_object(notedefs)) {
		error(errbuf, errsize, "notes must be an object");
		return 1;
	}

	json_object_foreach(notedefs, note, pair) {
		if (!json_is_array(pair) || json_array_size(pair) != 2 ||
		    !json_is_string(json_array_get(pair, 0)) ||
		    !json_is_number(json_array_get(pair, 1))) {
			error(errbuf, errsize, "note '%s' is defined incorrectly", note);
			return 1;
		}
	}
	return 0;
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
