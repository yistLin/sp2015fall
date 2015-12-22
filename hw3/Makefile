CC=gcc
OBJ=bidding_system customer bidding_system_EDF

all: $(s1) $(s2) $(c1)

s1: bidding_system

s2: bidding_system_EDF

c1: customer

bidding_system: bidding_system.c
	$(CC) -o $@ $<

bidding_system_EDF: bidding_system_EDF.c
	$(CC) -o $@ $<

customer: customer.c
	$(CC) -o $@ $<

clean:
	rm bidding_system bidding_system_EDF customer
