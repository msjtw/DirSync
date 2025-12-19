CC = gcc
CFLAGS = -g -I. -Wall
objects_comm = protocol.o
objects_s = server/connection.o server/main.o
objects_c = client/connection.o client/file_watch.o client/main.o

all: client server

client: $(objects_c) $(objects_comm) 
	$(CC) $(CFLAGS) -o client/client $^ 

server: $(objects_s) $(objects_comm) 
	$(CC) $(CFLAGS) -o server/server $^ 

$(objects_s) $(objects_c) $(objects_comm) : %.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


clean:
	rm  *.o server/*.o client/*.o server/server client/client
