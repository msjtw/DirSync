#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "../types.h"
#include "../protocol.h"


// Usage:
// gcc -Wall -g -o server server.c files.c
// ./server

void* thread_watch(void* arg) {
    printf("New thread - start: [%lu]", (unsigned long int)pthread_self());
    // int new_socket = *((int *)arg);

    while (1) {
        // TODO:
        // wait for changes from client
        // update files
        // send updated file to other clients
    }
    printf("Exit thread_watch \n");
    pthread_exit(NULL);
}


int main() {
    struct sockaddr_in sa;
    int server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1) {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof sa);

    sa.sin_family = AF_INET;
    sa.sin_port = htons(1100);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, (struct sockaddr*) &sa, sizeof sa) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 100) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_storage server_storage;
    pthread_t thread_id;

    while (1) {
        socklen_t addr_size = sizeof server_storage;
        int client_socket = accept(server_socket, (struct sockaddr *)&server_storage, &addr_size);
        if (client_socket == -1) {
            perror("Accept failed");
            close(server_socket);
            exit(EXIT_FAILURE);
        }

        // send a copy of current files to newly accepted client

        if (pthread_create(&thread_id, NULL, thread_watch, &client_socket) != 0) {
            printf("Failed to create thread\n");
        }
        pthread_detach(thread_id);
    }

    close(server_socket);
    return EXIT_SUCCESS;
}
