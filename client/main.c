#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
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

    int fw_socket = state.socket;

    struct pollfd pfds[2];
    pfds[0].fd = c_socket;
    pfds[0].events = POLLIN;
    pfds[1].fd = fw_socket;
    pfds[1].events = POLLIN;



    while (1) {
        int poll_count = poll(pfds, 2, -1);
        if (pfds[0].revents & POLLIN > 0){ // nonblocking read
            // read from server
            // recv_header()
            // if mess.type == NEW_FILE:  --> protocol.c
            //     recv_file()
        }
        if (pfds[1].revents & POLLIN > 0){ // nonblocking read
            // read form file change
            // ...
            // send file change
            // send_header() (what happend)
            // if NEW_FILE:
            //     send_file()
        }

    }
    

    
    // receive a copy from server
    // inotify -> send files after change (create, modify, delete)
    // including folders

    // receive change from server
    // add/delete file

    // TODO encrypting

    close(c_socket);
    return EXIT_SUCCESS;
}
