#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <jansson.h>

#include "temperament.h"
#include "util.h"

/**
 * A stack of note names, for use inside populate_notes.
 */
struct note_stack {
	const char *name;
	struct note_stack *next;
};

static struct note_stack *note_stack_push(struct note_stack *stack, const char *name);

static struct note_stack *note_stack_pop(struct note_stack *stack, const char **name);

static void error(char *errbuf, size_t errsize, char *fmt, ...);

static int populate(struct temperament *t, json_t *root, char *errbuf, size_t errsize);

static int validate_notes(json_t *notedefs, char *errbuf, size_t errsize);

/**
 * Populates the notes of the given temperament. Must be called after all
 * other values are populated.
 */
static int populate_notes(struct temperament *t, json_t *notedefs, char *errbuf, size_t errsize);

static int process_next_note(struct note_stack **todo, json_t *notedefs, struct notes *notes,
                             char *errbuf, size_t errsize);

/**
 * Attempts to assign the given offset to a note, failing if it conflicts
 * with the current definition.
 */
static int assign_offset(struct notes *notes, const char *name, double offset, char *errbuf, size_t errsize);

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

	retval = populate(&tmp, root, errbuf, errsize);
	if (retval)
		goto EXIT;

	*t = tmp;
	temperament_normalize(t);
	return retval;

EXIT:
	json_decref(root);
	return retval;
}

static struct note_stack *
note_stack_push(struct note_stack *stack, const char *name)
{
	struct note_stack *new;

	new = malloc(sizeof(*new));
	new->name = name;
	new->next = stack;
	return new;
}

static struct note_stack *
note_stack_pop(struct note_stack *stack, const char **name)
{
	struct note_stack *next;

	next = stack->next;
	*name = stack->name;
	free(stack);
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
populate(struct temperament *t, json_t *root, char *errbuf, size_t errsize)
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

	if (validate_notes(tmp, errbuf, errsize))
		return 1;

	return populate_notes(t, tmp, errbuf, errsize);
}

static int
validate_notes(json_t *notedefs, char *errbuf, size_t errsize)
{
	const char *note;
	json_t *pair;

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

static int
populate_notes(struct temperament *t, json_t *notedefs, char *errbuf, size_t errsize)
{
	struct notes *notes;
	struct note_stack *todo;

	notes = notes_alloc();
	notes_add(notes, t->reference_name, 0);
	todo = note_stack_push(NULL, t->reference_name);

	while (todo)
		if (process_next_note(&todo, notedefs, notes, errbuf, errsize)) {
			notes_free(notes);
			return 1;
		}

	t->notes = notes;
	return 0;
}

static int
process_next_note(struct note_stack **todo, json_t *notedefs, struct notes *notes,
                             char *errbuf, size_t errsize)
{
	const char *currnote, *newnote, *tmp;
	double curroffset, newoffset;
	json_t *pair;

	*todo = note_stack_pop(*todo, &currnote);

	notes_get_offset(notes, currnote, &curroffset);

	/* Check for the note on the left hand side. */
	pair = json_object_get(notedefs, currnote);
	if (pair) {
		newnote = json_string_value(json_array_get(pair, 0));
		newoffset = json_number_value(json_array_get(pair, 1));

		/*
		 * Make sure not to add the new note as a TODO if it's already
		 * defined. Even if it's already defined, though, we check the
		 * offset below to detect invalid input (multiple possible values
		 * for an offset).
		 */
		if (notes_get_offset(notes, newnote, NULL))
			*todo = note_stack_push(*todo, newnote);

		if (assign_offset(notes, newnote, curroffset - newoffset, errbuf, errsize))
			return 1;
	}

	/* Check for the note on the right hand side. */
	json_object_foreach(notedefs, newnote, pair) {
		tmp = json_string_value(json_array_get(pair, 0));
		if (!strcmp(tmp, currnote)) {
			newoffset = json_number_value(json_array_get(pair, 1));
			if (notes_get_offset(notes, newnote, NULL))
				*todo = note_stack_push(*todo, newnote);

			if (assign_offset(notes, newnote, curroffset + newoffset, errbuf, errsize))
				return 1;
		}
	}

	return 0;
}

static int
assign_offset(struct notes *notes, const char *name, double offset, char *errbuf, size_t errsize)
{
	double prevoffset;

	if (!notes_get_offset(notes, name, &prevoffset) &&
	    fmod(offset - prevoffset, CENTS_IN_OCTAVE) != 0) {
		error(errbuf, errsize, "found conflicting offset for '%s'", name);
		return 1;
	}
	notes_add(notes, name, offset);
	return 0;
}
