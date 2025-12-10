// File transfer functions
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

#include "files.h"

#define BUFF_SIZE 1024
#define END_MARKER "###END###"

int send_file(int client_socket, const char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return EXIT_FAILURE;    
    }

    char buff[BUFF_SIZE];
    ssize_t bytes_read;

    do {
        bytes_read = read(fd, buff, sizeof(buff));
        if (send(client_socket, buff, bytes_read, 0) == -1) {
            perror("Cannot send file");
            return EXIT_FAILURE;
        }
    } while (bytes_read > 0);
    
    close(fd);
    return EXIT_SUCCESS;
}


int receive_file(int server_socket, const char* filename) {
    // if filename exists, remove it
    // remove()

    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd == -1) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    char buff[BUFF_SIZE];
    ssize_t bytes_received;

    do {
        bytes_received = recv(server_socket, buff, sizeof(buff), 0);
        if (write(fd, buff, sizeof(buff)) == -1) {
            perror("Writing to file failed");
            return EXIT_FAILURE;
        }
    } while (bytes_received > 0);

    return EXIT_SUCCESS;
}

int receive_file_list(int server_socket) {
    char buff[BUFF_SIZE];

    while (1) {
        int bytes_received = recv(server_socket, buff, sizeof(buff), 0);
        if (bytes_received <= 0) {
            break;
        }

        if (strcmp(buff, END_MARKER) == 0) {
            break;
        }

        printf("Available file: %s\n", buff);
    }

    return EXIT_SUCCESS;
}