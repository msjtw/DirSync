#include "connection.h"
#include "file_watch.h"
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Usage:
// gcc -Wall -g -o ./client main.c connection.c file_watch.c
// ./client <address> <port> <path>

int main(int argc, char *argv[]) {
    // printf("Address: %s // Port: %d // Path: %s\n", argv[1], port, argv[3]);

    if (argc < 4) {
        printf("Missing input parametres (usage: ./client.out <address> <port> "
               "<path>)");
    }

    int c_socket = connection_init(argv[2], argv[1]);

    fw_state_t state;
    int msg_fd = fw_init(&state, argv[3]);

    struct pollfd pfds[3];
    pfds[0].fd = c_socket;
    pfds[0].events = POLLIN;
    pfds[1].fd = state.fd;
    pfds[1].events = POLLIN;
    pfds[2].fd = msg_fd;
    pfds[2].events = POLLIN;

    while (1) {
        int poll_count = poll(pfds, 2, -1);
        if (pfds[0].revents & POLLIN) { 
            // form server
            message_t msg;
            receive_message(c_socket, &msg);
        }
        if (pfds[1].revents & POLLIN) {
            // from inotify
            fw_handle_read(&state);
        }
        if (pfds[2].revents & POLLIN) { 
            // actions from inotify
            header_t hdr;
            read(pfds[2].fd, &hdr, sizeof hdr);
            if (hdr.type == NEW_FILE) {
                send_header(c_socket, &hdr);
                send_file(c_socket, hdr.path);
            }
            else{
                // NEW_DIR REMOVE
                send_header(c_socket, &hdr);
            }
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
