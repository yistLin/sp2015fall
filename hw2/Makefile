CC=gcc
CFLAGS=-std=c99 -Wall

all: bidding_system host player

bidding_system: bidding_system.c comb.o comb.h
	$(CC) $(CFLAGS) bidding_system.c comb.o -o bidding_system

comb.o: comb.c comb.h
	$(CC) $(CFLAGS) -c comb.c

host: host.c
	$(CC) $(CFLAGS) host.c -o host

player: player.c
	$(CC) $(CFLAGS) player.c -o player

clean:
	if [ -f bidding_system ]; then rm bidding_system; fi
	if [ -f host ]; then rm host; fi
	if [ -f player ]; then rm player; fi
	if [ -f comb.o ]; then rm comb.o; fi

