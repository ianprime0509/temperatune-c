enum { CENTS_IN_OCTAVE = 1200 };

/**
 * A complete description of a musical temperament.
 */
struct temperament {
	char *name;
	char *description;
	char *source;
	char *octave_base_name;
	double reference_pitch;
	char *reference_name;
	int reference_octave;
	struct notes *notes;
};

/**
 * Parses a temperament from the given input file. The strings and notes
 * struct within the temperament struct will be dynamically allocated,
 * and it is the responsibility of the caller to free them.
 *
 * As with the other functions, returns 0 on success. A buffer (and size)
 * can be provided for storing a textual representation of any error.
 */
int temperament_parse(struct temperament *t, FILE *input, char *errbuf, size_t errsize);

/**
 * Returns the pitch (in Hz) of the given note in the given octave, or
 * -1 if there is no such note.
 */
double temperament_get_pitch(const struct temperament *t, const char *note, int octave);

/**
 * Returns the name of the nearest note to the given pitch (in Hz). The
 * pointer should not be freed by the caller. If offset is not NULL, then the
 * offset (in cents) from the exact value of the note will be stored there.
 */
const char *temperament_nearest_note(const struct temperament *t, double pitch, double *offset);

/**
 * The notes of a temperament.
 */
struct notes;

/**
 * Adds the note with the given name and offset (in cents) from the reference
 * note. If the note is already defined, its definition is updated.
 */
int notes_add(struct notes *notes, const char *name, double offset);

struct notes *notes_alloc(void);

void notes_free(struct notes *notes);

/**
 * Returns a list of all note names as a dynamically allocated vector. Both
 * the strings in the vector and the vector itself must be freed by the
 * caller when no longer needed. The last element in the vector is always
 * NULL, for convenience.
 */
char **notes_get_names(const struct notes *notes);

/**
 * Finds the offset (in cents) of the note with the given name from the
 * reference note. Returns 0 on success (if the note was found). If offset
 * is not NULL, the offset will be stored there; otherwise, this function
 * is only good for checking the existence of a note.
 */
int notes_get_offset(const struct notes *notes, const char *name, double *offset);

/**
 * Returns the number of notes defined.
 */
size_t notes_size(const struct notes *notes);
