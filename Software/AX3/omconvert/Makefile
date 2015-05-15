#CC = gcc
CFLAGS = -O2 -Wall
LIBS = -lm
SRC = main.c omconvert.c butter4bp.c omcalibrate.c omdata.c linearregression.c wav.c calc-svm.c calc-csv.c calc-paee.c calc-wtv.c
INC =        omconvert.h butter4bp.h omcalibrate.h omdata.h linearregression.h wav.h calc-svm.h calc-csv.h calc-paee.h calc-wtv.h

all: omconvert

omconvert: Makefile $(SRC) $(INC)
	$(CC) -o omconvert $(CFLAGS) $(SRC) -I/usr/local/include -L/usr/local/lib $(LIBS)

clean:
	rm -f *.o core omconvert

