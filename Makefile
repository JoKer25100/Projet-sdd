CFLAGS = -g -Wno-unused-parameter -Wall -O2
PROGRAMS = main
OBJS = hash.o


.PHONY: all clean

all: $(PROGRAMS)

main: $(OBJS)
	gcc -o $@ $(CFLAGS) $^

hash.o: hash.c hash.h
	gcc $(CFLAGS) -c hash.c

clean:
	rm -f *.o *~ $(PROGRAMS)