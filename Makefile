OBJS=temperatune.o temperament.o temperament-parse.o util.o

temperatune: $(OBJS)
	$(CC) -o temperatune $(OBJS) -ljansson -lm

clean:
	rm -f temperatune $(OBJS)
