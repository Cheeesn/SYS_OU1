Flags = -g -std=gnu11 -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition 

all: mmake

mmake: mmake.o parser.o
	gcc $(Flags) -o mmake mmake.o parser.o
mmake.o: mmake.c parser.h
	gcc $(Flags) -c mmake.c
parser.o : parser.c parser.h
	gcc $(Flags) -c parser.c
clean:
	rm -f *.o

