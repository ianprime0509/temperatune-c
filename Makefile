CFLAGS=-Wall -Wextra -std=c99 -pedantic
CPPFLAGS=-D_XOPEN_SOURCE=700 -Iinclude

OBJS=\
	src/temperament.o \
	src/util.o

OBJS_ALL=$(OBJS) src/temperatune.o test/print.o

TESTPROGS=test/print

temperatune: $(OBJS) src/temperatune.o
	$(CC) $(CFLAGS) -o temperatune $(OBJS) src/temperatune.o -ljansson -lm

check: $(TESTPROGS) test/run.sh
	cd test && sh run.sh

clean:
	rm -f temperatune $(OBJS_ALL)

test/print: $(OBJS) test/print.o
	$(CC) $(CFLAGS) -o test/print $(OBJS) test/print.o -ljansson -lm
