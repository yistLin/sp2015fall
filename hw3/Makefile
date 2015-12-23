CC=gcc
OBJ=bidding_system customer bidding_system_EDF

all: $(OBJ)

bidding_system: bidding_system.c
	$(CC) -o $@ $<

bidding_system_EDF: bidding_system_EDF.c
	$(CC) -o $@ $<

customer: customer.c
	$(CC) -o $@ $<

clean:
	rm bidding_system bidding_system_EDF customer
