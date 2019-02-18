/*
 * Copyright (c) 2019 Ian Johnson
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

enum { OCTAVE_CENTS = 1200 };
enum { TABSIZE = 17 };

typedef struct Temperament Temperament;
typedef struct Note Note;
typedef Note *Notetab[TABSIZE];

struct Temperament {
	char *name;
	char *desc; /* description */
	char *src; /* source */
	char *octavebase; /* name of octave base note */
	double refpitch; /* reference pitch, in Hz */
	char *refname; /* name of reference note */
	int refoctave; /* octave number of reference note */
	Notetab notes;
};

const char *tfindnote(Temperament *t, double pitch, double *offset);
void tfreefields(Temperament *t);
double tgetpitch(Temperament *t, const char *note, int octave);
void tnormalize(Temperament *t);
int tparse(Temperament *t, FILE *input, char *errbuf, size_t errsize);

struct Note {
	char *name;
	double offset;
	Note *next;
};

void ntabadd(Notetab *ntab, const char *name, double offset);
void ntabfreenotes(Notetab *ntab);
int ntabget(Notetab *ntab, const char *name, double *offset);
size_t ntabsize(Notetab *ntab);
void ntabsortnames(Notetab *ntab, char *names[], size_t nnames);
void ntabstorenames(Notetab *ntab, char *names[]);
