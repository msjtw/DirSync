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
// gcc -Wall server.c -o ./server.out
// ./server.out

void thread_watch() {
    while (1) {
        // wait for changes from client
        // update files
        // send updated file to other clients
    }
    // detach/disconnect
};



int main(void)
{
    struct sockaddr_in sa;
    int server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1)
    {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof sa);

    sa.sin_family = AF_INET;
    sa.sin_port = htons(1100);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, (struct sockaddr *)&sa, sizeof sa) == -1) {
        perror("bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 100) == -1) {
        perror("listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    while (1) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1){
            perror("accept failed");
            close(server_socket);
            exit(EXIT_FAILURE);
        }

        // send a copy of current files to newly accepted client
        // add thread (-> function therad_watch)

        close(client_socket);
    }

    close(server_socket);
    return EXIT_SUCCESS;
}
