CFLAGS=-Wall -Wextra -std=c99 -pedantic
CPPFLAGS=-D_XOPEN_SOURCE=700

OBJS=temperatune.o temperament.o temperament-parse.o util.o

temperatune: $(OBJS)
	$(CC) $(CFLAGS) -o temperatune $(OBJS) -ljansson -lm

clean:
	rm -f temperatune $(OBJS)
