# Makefile
#
# With threads:  -lpthread

CC=gcc

all: build

build: Makefile waxrec.c
	$(CC) -o waxrec waxrec.c -lpthread

clean:
	rm -f *.o core
