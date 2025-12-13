#include "connection.h"
#include "file_watch.h"
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const int verbose = 1;
char path_pfx[256];

// Usage:
// ./client <address> <port> <path>

// TODO: when files are handled locally their relative paths are needed,
//       when file is send the path to the dir that's beeing watched needs to be stripped
//       on recv we need to add the path prefix back

int main(int argc, char *argv[]) {
    // printf("Address: %s // Port: %d // Path: %s\n", argv[1], port, argv[3]);

    if (argc < 4) {
        fprintf(stderr,
                "Missing input parametres (usage: ./client <address> <port> "
                "<path>)");
        exit(1);
    }

    int c_socket = connection_init(argv[2], argv[1]);

    strcpy(path_pfx, argv[3]);
    fw_state_t state;
    int msg_fd = fw_init(&state, path_pfx);


    struct pollfd pfds[4];
    pfds[0].fd = c_socket;
    pfds[0].events = POLLIN;
    pfds[1].fd = state.fd;
    pfds[1].events = POLLIN;
    pfds[2].fd = msg_fd;
    pfds[2].events = POLLIN;
    pfds[3].fd = 0; // STDIN
    pfds[3].events = POLLIN;

    char last_rcv_path[256];
    while (1) {
        int poll_count = poll(pfds, 4, -1);
        if (pfds[0].revents & POLLIN) {
            // form server
            printf("recv msg from server\n");
            message_t msg;
            receive_message(c_socket, &msg, path_pfx);
            strcpy(last_rcv_path, msg.header.path);
            free(msg.content);
        }
        if (pfds[1].revents & POLLIN) {
            // from inotify
            fw_handle_read(&state, path_pfx);
        }
        if (pfds[2].revents & POLLIN) {
            // actions from inotify
            header_t hdr;
            read(pfds[2].fd, &hdr, sizeof hdr);
            if (strcmp(last_rcv_path, hdr.path) == 0) {
                memset(&last_rcv_path, 0, sizeof last_rcv_path);
                printf("ingore change\n");
                continue;
            }
            if (hdr.type == MT_NEW_FILE) {
                // NEW_FILE
                printf("sending new file %s type: %u size: %lu\n", hdr.path, hdr.type, hdr.size);
                send_header(c_socket, &hdr);
                send_file(c_socket, hdr.path, path_pfx);
            } else {
                // NEW_DIR or REMOVE
                printf("sending new dir rm %s type %u size: %lu\n", hdr.path, hdr.type, hdr.size);
                send_header(c_socket, &hdr);
            }
        }
        if (pfds[3].revents & POLLIN) {
            char command[256];
            fgets(command, sizeof command, stdin);
            if (strcmp(command, "quit\n") == 0) {
                break;
            }
        }
    }

    fw_close(&state);
    close(c_socket);
    return EXIT_SUCCESS;
}
