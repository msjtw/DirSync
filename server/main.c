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
#include <poll.h>
#include "../protocol.h"
#include "connection.h"

#define MAX_CLIENTS 100

pthread_mutex_t message_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_mutex = PTHREAD_COND_INITIALIZER;

message_t current_message;
int total_clients = 0;
int client_sockets[MAX_CLIENTS];

// Usage (from 'server' directory):
// gcc -Wall -g -o server main.c connection.c ../protocol.c
// ./server <port>

void remove_client(int client_socket) {
    for (int i = 0; i < total_clients; i++) {
        if (client_sockets[i] == client_socket) {
            client_sockets[i] = client_sockets[total_clients-1];
            break;
        }
    }
    close(client_socket);
    total_clients--;
}

void free_message() {
    if (current_message.clients_sent == total_clients) {
        free(current_message.content);
        current_message.content = NULL;
        current_message.clients_sent = 0;
        pthread_cond_broadcast(&condition_mutex);
    }
}

/////////////////////////////////////////////////////////////////////////

void* client_thread(void *arg) {
    int client_socket = *((int*) arg);
    free(arg);

    // TODO send a copy of current files to newly accepted client



    int last_message_id = 0;

    while (1) {
        pthread_mutex_lock(&message_mutex);
        while (current_message.id == last_message_id) {
            pthread_cond_wait(&condition_mutex, &message_mutex);
        }
        struct message msg;
        msg.id = current_message.id;
        msg.header = current_message.header;
        msg.content = current_message.content;
        pthread_mutex_unlock(&message_mutex);

        /////////////////
        int n1 = send_header(client_socket, &msg.header);
        if (n1 < 0) {
            printf("Client disconnected %d during header send\n", client_socket);
            break;
        }

        if (msg.content && msg.header.size > 0) {
            int n2 = send_content(client_socket, msg.content, msg.header.size);
            if (n2 < 0) {
                printf("Client disconnected %d during message content send\n", client_socket);
                break;
            }
        }
        /////////////////

        pthread_mutex_lock(&message_mutex);
        current_message.clients_sent++;
        last_message_id = current_message.id;
        free_message();
        pthread_mutex_unlock(&message_mutex);
    }

    // Client disconnected
    pthread_mutex_lock(&message_mutex);
    remove_client(client_socket);
    free_message();
    pthread_mutex_unlock(&message_mutex);

    printf("Exited client %d thread\n", client_socket);
    pthread_exit(NULL);
}

/////////////////////////////////////////////////////////////////////////

void* receive_messages(void *arg) {

    struct pollfd poll_set[MAX_CLIENTS]; 

    while (1) {
        pthread_mutex_lock(&message_mutex);
        int count = total_clients;
        for (int i = 0; i < count; i++) {
            poll_set[i].fd = client_sockets[i];
            poll_set[i].events = POLLIN;
        }
        pthread_mutex_unlock(&message_mutex);


        int n1 = poll(poll_set, count, 5);
        if (n1 < 0) {
            perror("Poll error");
            continue;
        }
        else if (n1 == 0) { // No events
            continue;
        }

        for (int i = 0; i < count; i++) {
            if (!(poll_set[i].revents & POLLIN)) continue;
            message_t message;
            message.clients_sent = 0;
            message.content = NULL;

            int fd = poll_set[i].fd;
            int n2 = receive_message(fd, &message);
            if (n2 <= 0) {
                close(fd);
                perror("Receiving header failed");

                pthread_mutex_lock(&message_mutex);
                remove_client(fd);
                free_message();
                pthread_mutex_unlock(&message_mutex);
                continue;
            }

            header_t header = message.header;
            printf("Received new header from client: %u / %s\n", header.type, header.path);

            pthread_mutex_lock(&message_mutex);            
            while (current_message.clients_sent > 0 || current_message.content != NULL) {
                pthread_cond_wait(&condition_mutex, &message_mutex);
            }
            message.id = current_message.id + 1;
            current_message = message;
            pthread_cond_broadcast(&condition_mutex);
            pthread_mutex_unlock(&message_mutex);            
        }
    }
    
    printf("Exited 'receive_messages' thread\n");
    pthread_exit(NULL);
}


/////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Missing input parametres (usage: ./server <port>)\n");
        exit(1);
    }

    int server_socket = connection_init(argv[1], MAX_CLIENTS);

    /////////////////////////////////////////////
    // Receive messages

    pthread_t receive_thread_id;
    if (pthread_create(&receive_thread_id, NULL, receive_messages, NULL) != 0) {
        printf("Failed to create 'receive_messages' thread\n");
    }
    pthread_detach(receive_thread_id);


    /////////////////////////////////////////////
    // Accept new connections from clients

    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof client_addr;
    pthread_t thread_id;

    while (1) {
        int client_socket = accept(server_socket, (struct sockaddr*) &client_addr, &addr_size);
        if (client_socket == -1) {
            perror("Accept failed");
            close(server_socket);
            exit(EXIT_FAILURE);
        }

        pthread_mutex_lock(&message_mutex);
        client_sockets[total_clients] = client_socket;
        total_clients++;
        pthread_mutex_unlock(&message_mutex);

        int* p_client = malloc(sizeof(int));
        *p_client = client_socket;
        if (pthread_create(&thread_id, NULL, client_thread, p_client) != 0) {
            printf("Failed to create client thread\n");
            free(p_client);
            continue;
        }
        pthread_detach(thread_id);
    }

    close(server_socket);
    return EXIT_SUCCESS;
}
