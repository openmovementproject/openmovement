# Makefile for waxrec
# Dan Jackson, 2013
#

ifeq ($(OS),Windows_NT)
    UNAME := $(OS)
else
    UNAME := $(shell uname -s)
endif

ifeq ($(UNAME),Linux)
	# -lpthread -lm
	EXTRA_LIBS := 
else ifeq ($(UNAME),Windows_NT)
	# -lcfgmgr32
	EXTRA_LIBS := -lwsock32 -lsetupapi -lcfgmgr32
else
	# -lpthread -lm
	EXTRA_LIBS := 
endif

CC = gcc
CFLAGS = 
#CFLAGS = -I/usr/local/include -L/usr/local/lib 
LIBS = $(EXTRA_LIBS)
SRC = waxrec.c
INC = 

all: waxrec

waxrec: Makefile $(SRC) $(INC)
	$(CC) -o waxrec $(CFLAGS) $(SRC) $(LIBS)

clean:
	rm -f *.o core waxrec
