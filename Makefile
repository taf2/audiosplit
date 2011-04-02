LDFLAGS=`pkg-config sndfile --libs`
CFLAGS=-Wall -g `pkg-config sndfile --cflags`
all: splitter
splitter: splitter.o
splitter.o: splitter.c

clean:
	rm -f splitter.o splitter
