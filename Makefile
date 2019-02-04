CFLAGS=-Wall -Wextra -std=c99 -pedantic
CPPFLAGS=-D_XOPEN_SOURCE=700 -Iinclude

HEADERS=\
	include/temperament.h \
	include/util.h

OBJS_COMMON=\
	src/temperament.o \
	src/temperament-parse.o \
	src/util.o

OBJS=$(OBJS_COMMON) src/temperatune.o

OBJS_ALL=$(OBJS) test/print.o

TESTPROGS=test/print

temperatune: $(OBJS) $(HEADERS)
	$(CC) $(CFLAGS) -o temperatune $(OBJS) -ljansson -lm

check: $(TESTPROGS) test/run.sh
	cd test && sh run.sh

clean:
	rm -f temperatune $(OBJS_ALL)

test/print: $(OBJS_COMMON) test/print.o $(HEADERS)
	$(CC) $(CFLAGS) -o test/print $(OBJS_COMMON) test/print.o -ljansson -lm
