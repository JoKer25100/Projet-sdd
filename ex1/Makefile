CFLAGS = -g -Wno-unused-parameter -Wall -O2
PROGRAMS = test_hash test_handler
OBJS_HASH = hash.o test_hash.o
OBJS_HANDLER = hash.o Handler.o test_handler.o

.PHONY: all clean

all: $(PROGRAMS)

test_hash: $(OBJS_HASH)
	gcc -o $@ $(CFLAGS) $^

test_handler: $(OBJS_HANDLER)
	gcc -o $@ $(CFLAGS) $^

hash.o: hash.c hash.h
	gcc $(CFLAGS) -c hash.c

Handler.o: Handler.c Handler.h
	gcc $(CFLAGS) -c Handler.c

test_hash.o: test_hash.c hash.h
	gcc $(CFLAGS) -c test_hash.c

test_handler.o: test_handler.c hash.h Handler.h
	gcc $(CFLAGS) -c test_handler.c

clean:
	rm -f *.o *~ $(PROGRAMS)