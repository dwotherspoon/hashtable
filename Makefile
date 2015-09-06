CFLAGS=-Wall -Wextra -Wpedantic -O0 -I include/
CC=clang
EXECNAME=a.out

OBJ=main.o hashtable.o

all: $(OBJ)
	$(CC) -o $(EXECNAME) $(OBJ) 

clean:
	rm -f *.out
	rm -f *.o
	rm -f $(EXECNAME)
