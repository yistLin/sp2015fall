CC=gcc
CFLAGS=-std=c99 -Wall

all: bidding_system host player

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	if [ -f bidding_system ]; then rm bidding_system; fi
	if [ -f host ]; then rm host; fi
	if [ -f player ]; then rm player; fi

