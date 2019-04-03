CC = gcc
CFLAGS = -I.
DEPS = StreamParser.h FileStream.h
LIBS = -lm -ldl -lpthread
OBJ = convert.o StreamParser.o

all: convert

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

convert: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	-rm *.o convert
