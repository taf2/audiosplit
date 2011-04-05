LDFLAGS=`pkg-config sndfile --libs`
CFLAGS=-Wall -g `pkg-config sndfile --cflags`
all: splitter merge-wave
splitter: splitter.o
splitter.o: splitter.c splitter.h

merge-wave: merge-wave.o
merge-wave.o: merge-wave.c

clean:
	rm -f splitter.o splitter merge-wave merge-wave.o
