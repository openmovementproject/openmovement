#Linux OpenMovment Makefile
#by Ranjeet Singh

CC = gcc
CFLAGS = -w 
CFILES = main.c clear.c convert.c deploy.c download.c test.c verify.c 
INCLUDES = -I../include 
LIBS = -lomapi -ludev -lm

examples : $(CFILES)
	     $(CC) $(CFLAGS) $(INCLUDES) $(CFILES) -o examples -L. $(LIBS) 

