all: aesdsocket

re_socket.o: re_socket.c
	$(CC) $(CCFLAGS) -c re_socket.c
aesdsocket: re_socket.o
	$(CC) $(LDFLAGS) re_socket.o -o aesdsocket

# run the program
clean:
	rm -rf re_socket.o aesdsocket
