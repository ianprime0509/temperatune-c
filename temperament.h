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
