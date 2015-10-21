CC=gcc
DEPS= db_manager.h
OBJ= db_manager.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $<

all: $(OBJ) server.c
	gcc -o write_server $^
	gcc -D READ_SERVER $^ -o read_server

clean:
	rm -f read_server write_server $(OBJ)
