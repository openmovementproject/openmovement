# Java JNI Open Movement API Makefile

CC = gcc
CFLAGS = -w -c
LIBS = -lpthread -ludev 
OBJECTS = omapi-main.o omapi-internal.o omapi-download.o omapi-status.o omapi-reader.o omapi-settings.o
INCLUDES = -I../omapi/include

JOMAPI.so: JOMAPI.o $(OBJECTS)
	ld -shared -soname JOMAPI.so -o JOMAPI.so -lc JOMAPI.o $(OBJECTS)

JOMAPI.class: JOMAPI.java
	javac JOMAPI.java

JOMAPI.h: JOMAPI.class
	javah -classpath . -jni JOMAPI

JOMAPI.o: JOMAPI.c JOMAPI.h
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c JOMAPI.c -o JOMAPI.o

(%.o): ../omapi/src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c $< -o $*.o

clean:
	rm *.o
