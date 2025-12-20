#include "../protocol.h"
#include "connection.h"
#include <arpa/inet.h>
#include <bits/pthreadtypes.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_CLIENTS 100

pthread_mutex_t client_conn_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_mutex = PTHREAD_COND_INITIALIZER;

message_t message;
int total_clients = 0;
int client_sockets[MAX_CLIENTS];
int client_ports[MAX_CLIENTS];

void remove_client(int client_socket) {
    for (int i = 0; i < total_clients; i++) {
        if (client_sockets[i] == client_socket) {
            client_sockets[i] = client_sockets[total_clients - 1];
            break;
        }
    }
    close(client_socket);
    total_clients--;
}

/////////////////////////////////////////////////////////////////////////

static void client_disconneted(int fd) {
    pthread_mutex_lock(&client_conn_mutex);
    remove_client(fd);
    pthread_mutex_unlock(&client_conn_mutex);

    printf("Exited client %d thread\n", fd);
}

void *client_thread(void *arg) {
    int client_socket = *((int *)arg);

    /////////////////
    if (message.sender_fd != client_socket) {
        printf("Sending header - type: %d / client: %d / path: %s\n",
               message.header.type, client_socket,
               message.header.path);
        int header_nbytes = send_header(client_socket, &message.header);
        if (header_nbytes <= 0) {
            printf("Client %d disconnected during header send\n",
                   client_ports[client_socket]);
            client_disconneted(client_socket);
        }

        if (message.content && message.header.hsize > 0) {
            int content_nbytes = send_content(client_socket, &message);
            if(content_nbytes != message.header.hsize){
                printf("send %d instead of %d bytes", content_nbytes, message.header.hsize);
                exit(1);
            }
            if (content_nbytes <= 0) {
                printf("Client %d disconnected during message content send\n",
                       client_ports[client_socket]);
                client_disconneted(client_socket);
            }
        }
    }
    printf("send to %d\n", client_ports[client_socket]);
    pthread_exit(NULL);
}

/////////////////////////////////////////////////////////////////////////
// thre
void *receive_messages(void *arg) {

    struct pollfd poll_set[MAX_CLIENTS];

    while (1) {
        pthread_mutex_lock(&client_conn_mutex);
        int count = total_clients;
        for (int i = 0; i < count; i++) {
            poll_set[i].fd = client_sockets[i];
            poll_set[i].events = POLLIN;
        }
        pthread_mutex_unlock(&client_conn_mutex);

        int poll_cnt = poll(poll_set, count, 5);
        if (poll_cnt < 0) {
            perror("Poll error");
            continue;
        } else if (poll_cnt == 0) { // No events
            continue;
        }
        printf("-------------------\n");

        for (int i = 0; i < count; i++) {
            if (!(poll_set[i].revents & POLLIN))
                continue;
            int fd = poll_set[i].fd;


            memset(&message, 0, sizeof message);

            message.clients_sent = 0;
            message.content = NULL;
            message.sender_fd = fd;

            int rcv_status = receive_message(fd, &message, SERVER_STORAGE);
            printf("msg recvd\n");
            if (rcv_status <= 0) {
                close(fd);
                perror("Receiving header failed");

                pthread_mutex_lock(&client_conn_mutex);
                remove_client(fd);
                pthread_mutex_unlock(&client_conn_mutex);
                continue;
            }

            header_t header = message.header;
            printf(
                "Received new header from client %u type: %u / path: %s\n",
                client_ports[fd], header.type,
                header.path); // NOTE: to be fixed (client_ports[client_index])

            // send message co clients
            pthread_mutex_lock(&client_conn_mutex);
            int created_threads = total_clients;
            pthread_t *tids = malloc(total_clients * sizeof(pthread_t));
            for (int i = 0; i < total_clients; i++) {
                pthread_create(&tids[i], NULL, client_thread,
                               &client_sockets[i]);
            }
            pthread_mutex_unlock(&client_conn_mutex);
            for (int i = 0; i < created_threads; i++) {
                pthread_join(tids[i], NULL);
            }
            printf("all sends finished\n");
            free(tids);
            free(message.content);
        }
    }

    printf("Exited 'receive_messages' thread\n");
    pthread_exit(NULL);
}

/////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
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

    while (1) {
        int client_socket =
            accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket == -1) {
            perror("Accept failed");
            continue;
        }

        pthread_mutex_lock(&client_conn_mutex);
        client_sockets[total_clients] = client_socket;
        client_ports[client_socket] = ntohs(client_addr.sin_port);
        total_clients++;
        pthread_mutex_unlock(&client_conn_mutex);
    }

    close(server_socket);
    return EXIT_SUCCESS;
}
