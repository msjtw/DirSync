// Connection init for server
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../protocol.h"

int connection_init(const char *port, int max_clients) {
    struct sockaddr_in sa;
    int server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1) {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }

    const int enable = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
    }

    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(strtol(port, 0, 10));
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, (struct sockaddr *)&sa, sizeof sa) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, max_clients) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Starting server on port %s...\n", port);
    printf("Server files will be stored in: %s\n", SERVER_STORAGE);
    
    if (mkdir(SERVER_STORAGE, 0777) == -1) {
        perror("Failed to create server storage directory");
    }

    return server_socket;
}
