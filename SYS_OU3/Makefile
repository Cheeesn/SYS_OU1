# Compiler and flags
CC = gcc
Flags = -g -std=gnu11 -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition

# Targets
all: mdu

# Link mdu executable
mdu: mdu.o
	$(CC) -lpthread -lm $(Flags) -o mdu mdu.o

# Compile source file into object file
mdu.o: mdu.c
	$(CC) $(Flags) -c -o mdu.o mdu.c

# Clean up compiled files
clean:
	rm -f *.o mdu
