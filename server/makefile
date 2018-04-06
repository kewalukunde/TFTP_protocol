cflags= -Wall

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

server : $(OBJ)
	gcc -o $@ $^

$(OBJ) : $(SRC)
	gcc -Wall $(cflags) -I. -c $^

clean :
	rm *.o server
