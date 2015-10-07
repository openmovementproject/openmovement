# detect OS
ifeq ($(OS),Windows_NT)
  UNAME := $(OS)
else
  UNAME := $(shell uname -s)
endif

ifeq ($(UNAME),Linux)
  LIBS := -lm
else ifeq ($(UNAME),Windows_NT)
  LIBS := -lwsock32 -lcfgmgr32
else
  LIBS := -lm
endif

TARGET = bin2csv
CC = gcc
CFLAGS = -g -Wall -Wno-attributes -Wno-unused-function $(OPTFLAGS)

MKDIR=$(shell mkdir -p obj)

.PHONY: default all clean

default: $(TARGET)
all: $(MKDIR) default

OBJECTS = $(patsubst %.c, obj/%.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

obj/%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -g -Wall $(LIBS) -o $@

clean:
	-rm -rf obj/
	-rm -f $(TARGET)
