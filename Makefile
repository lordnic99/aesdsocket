#
# In order to execute this "Makefile" just type "make"
#	A. Delis (ad@di.uoa.gr)
#

OBJS	?= re_socket.o
SOURCE	?= re_socket.c
HEADER	= 
OUT	?= aesdsocket
CC	 ?= gcc
FLAGS	 ?= -g -c -Wall
LFLAGS	 = -lpthread
# -g option enables debugging mode 
# -c flag generates object code for separate files


all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)


# create/compile the individual files >>separately<<
socket.o: re_socket.c
	$(CC) $(FLAGS) re_socket.c 


# clean house
clean:
	rm -f $(OBJS) $(OUT)


# run the program
run: $(OUT)
	./$(OUT) -d
