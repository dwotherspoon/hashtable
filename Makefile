CFLAGS=-Wall -Wextra -Wpedantic -g -O0 -I include/ -pg
CC=clang
EXECNAME=a.out

OBJ=main.o hashtable.o

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(EXECNAME) $(OBJ) 

clean:
	rm -f *.out
	rm -f *.o
	rm -f $(EXECNAME)
