# Java JNI Open Movement API Makefile

CC = gcc
CFLAGS = -w -c
LIBS = -lpthread -ludev 
OBJECTS = omapi-main.o omapi-internal.o omapi-download.o omapi-status.o omapi-reader.o omapi-settings.o
INCLUDES = -I../omapi/include

lib/JOMAPI.jar: class/openmovement/JOMAPI.class bin/JOMAPI.so
	mkdir lib
	jar cvfe lib/JOMAPI.jar openmovement.JOM -C class openmovement/JOMAPI.class -C class openmovement/JOM.class -C class openmovement/JOMAPIListener.class

bin/JOMAPI.so: JOMAPI.o $(OBJECTS)
	bin lib
	ld -shared -soname bin/JOMAPI.so -o JOMAPI.o -lc JOMAPI.o $(OBJECTS)

JOMAPI.o: c/JOMAPI.c c/openmovement_JOMAPI.h
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c c/JOMAPI.c -o JOMAPI.o

c/openmovement_JOMAPI.h: class/openmovement/JOMAPI.class
	javah -classpath class -d c -jni openmovement.JOMAPI

class/openmovement/JOMAPI.class: src/openmovement/JOMAPI.java src/openmovement/JOM.java src/openmovement/JOMAPIListener.java
	mkdir class
	javac -classpath class -d class -sourcepath src src/openmovement/JOM.java

(%.o): ../omapi/src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c $< -o $*.o

clean:
	rm *.o
