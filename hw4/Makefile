CC=g++
CFLAGS=-lpthread
OBJ=merger

all: $(OBJ)

merger: merger.cpp
	$(CC) $(CFLAGS) -o $@ $<

run:
	./merger 4 < testdata

clean:
	rm -f merger
