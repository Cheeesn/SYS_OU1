Flags = -g -std=gnu11 -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition

        
all: 
	gcc $(Flags) mexec.c -o mexec
	
