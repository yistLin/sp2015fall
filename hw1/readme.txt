# SP HW1

**Warning! This readme.txt is written in Markdown**

Student ID: B03902048

### How to compile?

type `make` to compile out *read_server* and *write_server*

### How to run?

type `./read_server [port number]` to run read server

type `./write_server [port number]` to run write server

### How did I finish my program?

* First I try to write a simple *db_manager.h* and *db_manager.c* to read *item_list* and print it out
* Later, I search a lot of information to help me understand what is `select()`, and how to use it
* After I have simply knowledge about `select()`, I try to inplement I/O multiplexing while read request come in
* Then, I finish the rest part of IO multiplexing step by step
* Later, I used `fcntl()` to inplement region file lock in my `db_manager.c`
* I was stuck by a big problem when inplemented file lock when two client connected to the same *write_server*
* I tried so many method and browser a great deal of website and manual page to find what cause this problem
* After that, I only know that *fcntl()* works process-associated, but still don't how to fix it
* Then, I read *server.c* more carefully, and I found there are `int item;` and `int wait_to_write;`!
* Finally, I solved this problem by checking these when receive write request

