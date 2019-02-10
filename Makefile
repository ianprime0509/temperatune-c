CFLAGS=-Wall -Wextra -std=c99 -pedantic
CPPFLAGS=-D_XOPEN_SOURCE=700 -Iinclude
LIBS=-lportaudio -ljansson -lm -lpthread

OBJS=\
	src/audio.o \
	src/temperament.o \
	src/util.o

OBJS_ALL=$(OBJS) src/ttplay.o test/print.o

TESTPROGS=test/print

temperatune: $(OBJS) src/ttplay.o
	$(CC) $(CFLAGS) -o ttplay $(OBJS) src/ttplay.o $(LIBS)

check: $(TESTPROGS) test/run.sh
	cd test && sh run.sh

clean:
	rm -f ttplay $(OBJS_ALL)

test/print: $(OBJS) test/print.o
	$(CC) $(CFLAGS) -o test/print $(OBJS) test/print.o -ljansson -lm
