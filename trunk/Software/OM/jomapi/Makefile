# Java JNI Open Movement API Makefile

CC = gcc
CFLAGS = -w -c
LIBS = -lpthread -ludev 
OBJECTS = omapi-main.o omapi-internal.o omapi-download.o omapi-status.o omapi-reader.o omapi-settings.o
INCLUDES = -I../omapi/include

JOMAPI.so: JOMAPI.o $(OBJECTS)
	ld -shared -soname JOMAPI.so -o JOMAPI.so -lc JOMAPI.o $(OBJECTS)

JOMAPI.class: src/openmovement/JOMAPI.java
	javac -classpath class -sourcepath src src/openmovement/JOMAPI.java

c/openmovement_JOMAPI.h: class/openmovement/JOMAPI.class
	javah -classpath class -d c -jni openmovement.JOMAPI

JOMAPI.o: c/JOMAPI.c c/openmovement_JOMAPI.h
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c c/JOMAPI.c -o JOMAPI.o

(%.o): ../omapi/src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c $< -o $*.o

clean:
	rm *.o
