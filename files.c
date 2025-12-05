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


int send_file_list(int client_socket, const char* folder_path) {
    DIR* directory = opendir(folder_path);
    if (directory == NULL) {
        perror("Error opening directory");
        return EXIT_FAILURE;
    }

    struct dirent* entry;
    do {
        entry = readdir(directory);
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (send(client_socket, entry->d_name, strlen(entry->d_name)+1, 0) == -1) {
            perror("Error sending file name");
            break;
        }
    } while (entry != NULL);

    send(client_socket, END_MARKER, strlen(END_MARKER)+1, 0);
    closedir(directory);
    return EXIT_SUCCESS;
}


// int send_directory(int client_socket, char path[]) {
//     DIR* fd;
// }

int send_directory(int client_socket, const char* path) {
    printf("Directory tree - entered: %s\n", path);
    DIR* fd = opendir(path);

    if (fd == NULL) {
        perror("Failed to open target directory");
        return EXIT_FAILURE;
    }

    struct dirent* in_file;
    do {
        in_file = readdir(fd);

        if (in_file->d_type == DT_DIR) {
            if (!(strcmp(".", in_file->d_name) == 0 || strcmp("..", in_file->d_name) == 0)){
                char *dir_path = malloc(strlen(path) + strlen(in_file->d_name) + 3);
                strcpy(dir_path, path);
                strcat(dir_path, "/");
                strcat(dir_path,  in_file->d_name);
                send_directory(client_socket, dir_path);
                free(dir_path);
            }
        }

        if (in_file->d_type == DT_REG) {
            printf("File: %s\n", in_file->d_name);
            send_file(client_socket, in_file->d_name);
        }

    } while (in_file);

    closedir(fd);
    return EXIT_SUCCESS;
}

int receive_file(int server_socket, const char* filename) {
    int fd = open(filename, O_WRONLY | O_CREAT);
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