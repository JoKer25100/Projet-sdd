CFLAGS = -g -Wno-unused-parameter -Wall -O2
PROGRAMS = test_hash test_handler test_parser test_cpu
OBJS_HASH = hash.o test_hash.o
OBJS_HANDLER = hash.o Handler.o test_handler.o
OBJS_PARSER = hash.o Handler.o Parser.o test_parser.o
OBJ_CPU = Cpu.o hash.o Handler.o Parser.o test_parser.o

.PHONY: all clean

all: $(PROGRAMS)

test_cpu: $(OBJ_CPU)
	gcc -o $@ $(CFLAGS) $^

test_hash: $(OBJS_HASH)
	gcc -o $@ $(CFLAGS) $^

test_handler: $(OBJS_HANDLER)
	gcc -o $@ $(CFLAGS) $^

test_parser: $(OBJS_PARSER)
	gcc -o $@ $(CFLAGS) $^

hash.o: hash.c hash.h
	gcc $(CFLAGS) -c hash.c

Handler.o: Handler.c Handler.h
	gcc $(CFLAGS) -c Handler.c

Parser.o: Parser.c Parser.h Handler.h hash.h
	gcc $(CFLAGS) -c Parser.c

Cpu.o: Cpu.c Cpu.h hash.h Handler.h Parser.h
	gcc $(CFLAGS) -c Cpu.c

test_hash.o: test_hash.c hash.h
	gcc $(CFLAGS) -c test_hash.c

test_handler.o: test_handler.c hash.h Handler.h
	gcc $(CFLAGS) -c test_handler.c

test_parser.o: test_parser.c hash.h Handler.h Parser.h
	gcc $(CFLAGS) -c test_parser.c

test_cpu.o: test_cpu.c Cpu.h hash.h Handler.h Parser.h
	gcc $(CFLAGS) -c test_cpu.c
clean:
	rm -f *.o *~ $(PROGRAMS)