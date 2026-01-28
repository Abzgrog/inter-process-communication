CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_GNU_SOURCE

all: zadanie proc_p1 proc_p2 proc_t proc_d proc_serv2

zadanie: zadanie.c
	$(CC) $(CFLAGS) zadanie.c -o zadanie
	
proc_p1: proc_p1.c
	$(CC) $(CFLAGS) proc_p1.c -o proc_p1

proc_p2: proc_p2.c
	$(CC) $(CFLAGS) proc_p2.c -o proc_p2

proc_t: proc_t.c
	$(CC) $(CFLAGS) proc_t.c -o proc_t

proc_d: proc_d.c
	$(CC) $(CFLAGS) proc_d.c -o proc_d

proc_serv2: proc_serv2.c
	$(CC) $(CFLAGS) proc_serv2.c -o proc_serv2

clean:
	rm -f zadanie proc_p1 proc_p2 proc_t proc_d proc_serv2 serv2.txt