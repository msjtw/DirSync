CC = gcc
CFALGS = -g -I.
objects_comm = protocol.o
objects_s = server/connection.o 
objects_c = client/connection.o client/file_watch.o

all: client server

client: $(objects_c) $(objects_comm)
	$(CC) $(CFLAGS) -o client/client $^ client/main.c

server: $(objects_s) $(objects_comm)
	$(CC) $(CFLAGS) -o server/server $^ server/main.c

$(objects_s) $(objects_c) $(objects_comm) : %.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^


clean:
	rm  *.o server/*.o client/*.o server/server client/client
