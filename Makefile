CC=gcc
IDIR=./headers
LIBS=-lgsl -lgslcblas -lm 
LFLAGS=$(LIBS) -I$(IDIR)
CFLAGS=-Wall -static

all: spectre

spectre: main.c 
	$(CC) $(CFLAGS) *.c -o ./build/main $(LFLAGS) 
