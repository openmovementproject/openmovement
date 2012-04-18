#Linux OpenMovment Makefile
#by Ranjeet Singh

CC = gcc
CFLAGS = -w -c
LIBS = -lpthread -ludev 
OBJECTS = omapi-main.o omapi-internal.o omapi-download.o omapi-status.o omapi-reader.o omapi-settings.o
INCLUDES = -I../include

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

clean:
	rm *.o
