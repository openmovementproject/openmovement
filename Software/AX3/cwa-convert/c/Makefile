CC = gcc
CFLAGS = -I.
DEPS = cwa.h sqlite3.h
LIBS = -lm -ldl -lpthread
OBJ = main.o cwa.o sqlite3.o

all: cwa-convert

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

cwa-convert: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	-rm *.o cwa-convert
