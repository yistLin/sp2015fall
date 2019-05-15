CC=gcc
DEPS=db_manager.h
OBJ=main.o db_manager.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $<

all: $(OBJ)
	gcc -o test $^

clean:
	rm -f $(OBJ) test

