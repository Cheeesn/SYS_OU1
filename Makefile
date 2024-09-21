Flags = -g -std=gnu11 -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition

mexec: mexec.o
	gcc $(Flags) mexec.c -o mexec
mexec.o: mexec.c
	gcc $(Flags) -c -o mexec.o mexec.c
        
all: mexec
	
