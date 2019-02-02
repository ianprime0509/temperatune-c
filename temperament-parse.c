#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <jansson.h>

#include "temperament.h"
#include "util.h"

static void error(char *errbuf, size_t errsize, char *fmt, ...);

/**
 * Populates the given temperament from the given JSON.
 */
static int temperament_populate(struct temperament *t, json_t *root, char *errbuf, size_t errsize);

/**
 * Returns whether the given JSON is a valid note+offset pair.
 */
static int is_valid_pair(json_t *pair);

/**
 * Populates the notes of the given temperament. Must be called after all
 * other values are populated.
 */
static int temperament_populate_notes(struct temperament *t, json_t *notes, char *errbuf, size_t errsize);

int
temperament_parse(struct temperament *t, FILE *input, char *errbuf, size_t errsize)
{
	json_t *root;
	json_error_t err;
	struct temperament tmp;
	int retval;

	retval = 0;
	root = json_loadf(input, 0, &err);
	if (!root) {
		error(errbuf, errsize, "could not parse input: %s", err.text);
		retval = 1;
		goto EXIT;
	}

	retval = temperament_populate(&tmp, root, errbuf, errsize);
	if (retval)
		goto EXIT;

	*t = tmp;
	return retval;

EXIT:
	json_decref(root);
	return retval;
}

static void error(char *errbuf, size_t errsize, char *fmt, ...)
{
	va_list args;

	if (!errbuf)
		return;
	va_start(args, fmt);
	vsnprintf(errbuf, errsize, fmt, args);
	va_end(args);
}

static int
temperament_populate(struct temperament *t, json_t *root, char *errbuf, size_t errsize)
{
	json_t *tmp;
	const char *str;
	double d;

	if (!json_is_object(root)) {
		error(errbuf, errsize, "input is not a JSON object");
		return 1;
	}

	if (!(str = json_string_value(json_object_get(root, "name")))) {
		error(errbuf, errsize, "name not found");
		return 1;
	}
	t->name = xstrdup(str);

	if ((str = json_string_value(json_object_get(root, "description"))))
		t->description = xstrdup(str);
	else
		t->description = NULL;

	if ((str = json_string_value(json_object_get(root, "source"))))
		t->source = xstrdup(str);
	else
		t->source = NULL;

	if (!(str = json_string_value(json_object_get(root, "octaveBaseName")))) {
		error(errbuf, errsize, "octave base name not found");
		return 1;
	}
	t->octave_base_name = xstrdup(str);

	tmp = json_object_get(root, "referencePitch");
	if (!json_is_number(tmp)) {
		error(errbuf, errsize, "reference pitch not found");
		return 1;
	}
	d = json_number_value(tmp);
	if (d <= 0) {
		error(errbuf, errsize, "reference pitch must be greater than zero");
		return 1;
	}
	t->reference_pitch = d;

	if (!(str = json_string_value(json_object_get(root, "referenceName")))) {
		error(errbuf, errsize, "reference note name not found");
		return 1;
	}
	t->reference_name = xstrdup(str);

	tmp = json_object_get(root, "referenceOctave");
	if (!json_is_integer(tmp)) {
		error(errbuf, errsize, "reference octave not found");
		return 1;
	}
	t->reference_octave = json_integer_value(tmp);

	if (!(tmp = json_object_get(root, "notes"))) {
		error(errbuf, errsize, "notes not found");
		return 1;
	}

	return temperament_populate_notes(t, tmp, errbuf, errsize);
}

static int
is_valid_pair(json_t *pair)
{
	return json_is_array(pair) &&
	    json_is_string(json_array_get(pair, 0)) &&
	    json_is_number(json_array_get(pair, 1));
}

static int
temperament_populate_notes(struct temperament *t, json_t *notedefs, char *errbuf, size_t errsize)
{
	struct notes *notes;
	json_t *pair;

	notes = notes_alloc();
	notes_add(notes, t->reference_name, 0);

	pair = json_object_get(notedefs, t->reference_name);
	if (!is_valid_pair(pair)) {
		error(errbuf, errsize, "reference note is not defined as a note");
		goto FAIL;
	}

	return 0;

FAIL:
	notes_free(notes);
	return 1;
}
