#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../types.h"
#include "connection.h"
#include "file_watch.h"

// Usage:
// gcc -Wall -g -o ./client main.c connection.c file_watch.c
// ./client <address> <port> <path>

int main(int argc, char *argv[])
{
    // printf("Address: %s // Port: %d // Path: %s\n", argv[1], port, argv[3]);

    if (argc < 4) {
        printf("Missing input parametres (usage: ./client.out <address> <port> <path>)");
    }

    int c_socket = connection_init(argv[2], argv[1]);

    fw_state_t state;
    fw_init(&state, argv[3]);

    
    // receive a copy from server
    // inotify -> send files after change (create, modify, delete)
    // including folders

    // receive change from server
    // add/delete file

    // TODO encrypting

    close(c_socket);
    return EXIT_SUCCESS;
}
