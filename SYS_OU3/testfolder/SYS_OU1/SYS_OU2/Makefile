Flags = -g -std=gnu11 -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition 

mmake: mmake.o parser.o
	gcc $(Flags) mmake.c -o mmake parser.o
mmake.o: mmake.c parser.h
	gcc $(Flags) -c mmake.c
parser.o : parser.c
	gcc $(Flags) -c parser.c
clean:
	rm -f *.o mmake parser.o

