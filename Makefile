CFLAGS=-Wall -Wextra -std=c99 -pedantic
CPPFLAGS=-D_XOPEN_SOURCE=700
LIBS=-lportaudio -ljansson -lm

OBJS=audio.o temperament.o util.o

TESTPROGS=test/print

temperatune: $(OBJS) ttplay.o
	$(CC) $(CFLAGS) -o ttplay $(OBJS) ttplay.o $(LIBS)

check: $(TESTPROGS) test/run.sh
	cd test && sh run.sh

clean:
	rm -f ttplay test/print $(OBJS) ttplay.o test/print.o

test/print: $(OBJS) test/print.o
	$(CC) $(CFLAGS) -I. -o test/print $(OBJS) test/print.o $(LIBS)
