CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic

all: encode decode entropy

encode: encode.o 
	$(CC) -o encode encode.o huffman.o io.o node.o stack.o pq.o code.o

encode.o:
	$(CC) $(CFLAGS) -c encode.c huffman.c io.c node.c stack.c pq.c code.c

decode: decode.o 
	$(CC) -o decode decode.o huffman.o io.o node.o stack.o pq.o code.o

decode.o:
	$(CC) $(CFLAGS) -c decode.c huffman.c io.c node.c stack.c pq.c code.c

entropy: entropy.o
	$(CC) -o entropy entropy.o -lm

entropy.o:
	$(CC) $(CFLAGS) -c entropy.c

format:
	clang-format -i -style=file *.c *.h

clean:
	rm -f encode decode entropy ./*.o

scan-build: clean
	scan-build make
