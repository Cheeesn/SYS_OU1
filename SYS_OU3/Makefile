Flags = -g -std=gnu11 -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition 

all: mdu

mexec: mdu.o
	gcc $(Flags) mdu.c -o mdu
mexec.o: mdu.c
	gcc $(Flags) -c -o mdu.o mdu.c
        
clean:
	rm -f *.o mdu
