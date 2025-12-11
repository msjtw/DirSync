// Connection protocol functions
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "protocol.h"

#define BUFF_SIZE 1024

/////////////////////////////////////////////////////////////

int send_header(int sock, header_t* header) {
    int n = send(sock, &header, sizeof header, 0);
    if (n < 0) {
        perror("Send header - error");
    }
    return n;
}

int send_content(int sock, const void* buf, size_t length) {
    ssize_t sent = 0;
    while (sent < length) {
        ssize_t n = send(sock, (char*)buf, length - sent, 0);
        if (n <= 0) return -1;
        sent += n;
    }
    return EXIT_SUCCESS;
}

int send_file(int sock, const char* filename) {
    int file = open(filename, O_RDONLY);
    if (file == -1) {
        perror("Error opening file");
        return EXIT_FAILURE;    
    }

    char buff[BUFF_SIZE];
    ssize_t bytes_read;

    do {
        bytes_read = read(file, buff, sizeof(buff));
        if (send(sock, buff, bytes_read, 0) == -1) {
            perror("Cannot send file");
            return EXIT_FAILURE;
        }
    } while (bytes_read > 0);
    
    close(file);
    return EXIT_SUCCESS;
}

/////////////////////////////////////////////////////////////

int receive_header(int sock) {
    header_t header;
    
    int n = recv(sock, &header, sizeof header, 0);
    if (n > 0) {
        if (header.type == NEW_FILE) {
            receive_file(sock, header.path);
        }
        else if (header.type == NEW_DIR) {
            mkdir(header.path, 0777);
        }
        else if (header.type == REMOVE) {
            remove(header.path);
        }
    }
    else if (n < 0) {
        perror("Receive header - error");
    } 
    return n;
}

int receive_file(int sock, const char* filename) {

    int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC);
    if (file == -1) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    char buff[BUFF_SIZE];
    ssize_t bytes_received;

    do {
        bytes_received = recv(sock, buff, sizeof(buff), 0);
        if (write(file, buff, sizeof(buff)) == -1) {
            perror("Writing to file failed");
            return EXIT_FAILURE;
        }
    } while (bytes_received > 0);

    return EXIT_SUCCESS;
}