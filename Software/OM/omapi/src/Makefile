CC = gcc
CFLAGS = -w -c
LIBS = 
OBJECTS = omapi-main.o omapi-internal.o omapi-download.o omapi-status.o omapi-reader.o omapi-settings.o
INCLUDES = -I../include

ifeq ($(OS),Windows_NT)
	OBJECTS += omapi-devicefinder-win.o
else
	UNAME_R := $(shell uname -r | grep -o Microsoft)
	ifeq ($(UNAME_R),Microsoft)
		# WSL Not supported
		OBJECTS += omapi-devicefinder-linux.o
		LIBS += -lpthread -ludev
	else
		UNAME_S := $(shell uname -s)
		ifeq ($(UNAME_S),Linux)
			OBJECTS += omapi-devicefinder-linux.o
			LIBS += -lpthread -ludev
		endif
		ifeq ($(UNAME_S),Darwin)
			OBJECTS += omapi-devicefinder-mac.o
			LIBS += -lpthread
		endif
	endif
endif

libomapi.a : $(OBJECTS)
	ar rcs libomapi.a $(OBJECTS)

omapi-main.o : omapi-main.c
	$(CC) $(CFLAGS) $(INCLUDES) -c omapi-main.c $(LIBS)

omapi-internal.o : omapi-internal.c
	$(CC) $(CFLAGS) $(INCLUDES) -c omapi-internal.c $(LIBS)

omapi-download.o : omapi-download.c
	$(CC) $(CFLAGS) $(INCLUDES) -c omapi-download.c $(LIBS)

omapi-status.o : omapi-status.c
	$(CC) $(CFLAGS) $(INCLUDES) -c omapi-status.c $(LIBS)

omapi-reader.o : omapi-reader.c
	$(CC) $(CFLAGS) $(INCLUDES) -c omapi-reader.c $(LIBS)

omapi-settings.o : omapi-settings.c
	$(CC) $(CFLAGS) $(INCLUDES) -c omapi-settings.c $(LIBS)

omapi-devicefinder-linux.o : omapi-devicefinder-linux.c
	$(CC) $(CFLAGS) $(INCLUDES) -c omapi-devicefinder-linux.c $(LIBS)

omapi-devicefinder-mac.o : omapi-devicefinder-mac.c
	$(CC) $(CFLAGS) $(INCLUDES) -c omapi-devicefinder-mac.c $(LIBS)

omapi-devicefinder-win.o : omapi-devicefinder-win.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c omapi-devicefinder-win.cpp $(LIBS)

clean:
	rm -f *.o libomapi.a
