#CC = gcc
CFLAGS = -O2 -Wall
LIBS = -lm
SRC = butter.c calc-csv.c calc-paee.c calc-sleep.c calc-svm.c calc-wtv.c linearregression.c main.c omcalibrate.c omconvert.c omdata.c wav.c
INC = butter.h calc-csv.h calc-paee.h calc-sleep.h calc-svm.h calc-wtv.h exits.h linearregression.h omcalibrate.h omconvert.h omdata.h wav.h

all: omconvert

omconvert: Makefile $(SRC) $(INC)
	$(CC) -o omconvert $(CFLAGS) $(SRC) -I/usr/local/include -L/usr/local/lib $(LIBS)

clean:
	rm -f *.o core omconvert


