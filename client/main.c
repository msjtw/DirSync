#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "types.h"

// Usage:
// gcc -Wall client.c -o ./client.out
// ./client.out <address> <port> <path>

int main(int argc, char *argv[])
{
    struct sockaddr_in sa;

    if (argc < 4) {
        printf("Missing input parametres (usage: ./client.out <address> <port> <path>)");
    }

    int port = atoi(argv[2]);
    printf("Address: %s // Port: %d // Path: %s\n", argv[1], port, argv[3]);
    int client_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == -1) {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof sa);

    sa.sin_addr.s_addr = inet_addr(argv[1]);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

    if (connect(client_socket, (struct sockaddr*) &sa, sizeof sa) == -1) {
        perror("Connect failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connection accepted \n");

    // receive a copy from server
    // inotify -> send files after change (create, modify, delete)
    // including folders

    // receive change from server
    // add/delete file

    // TODO encrypting

    close(client_socket);
    return EXIT_SUCCESS;
}
