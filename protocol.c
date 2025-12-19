// Connection protocol functions
#include "protocol.h"
#include <arpa/inet.h>
#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

/////////////////////////////////////////////////////////////

static int receive_file(int sock, const char *filename, char *buff, int bsziem,
                        char *path_pfx);
static char *path_concat(const char *path, const char *name);

int send_header(const int sock, const header_t *header) {
    int n = send(sock, header, sizeof(header_t), 0);
    if (n < 0) {
        perror("Send header - error");
    }
    return n;
}

int send_content(const int sock, const message_t *msg) {
    ssize_t sent = 0;
    while (sent < msg->header.hsize) {
        size_t n = send(sock, msg->content+sent, msg->header.hsize - sent, 0);
        if (n <= 0)
            return -1;
        sent += n;
    }
    return sent;
}

int send_file(const int sock, const char *filename, const char *path_pfx) {
    char *path = path_concat(path_pfx, filename);
    int file = open(path, O_RDONLY);
    if (file == -1) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    char buff[256];
    size_t bytes_read;
    size_t sendc = 0;


    do {
        bytes_read = read(file, buff, sizeof(buff));
        sendc += bytes_read;
        if (send(sock, buff, bytes_read, 0) == -1) {
            perror("Cannot send file");
            return EXIT_FAILURE;
        }
    } while (bytes_read > 0);

    free(path);
    close(file);
    return sendc;
}

int send_dir_tree(const int sock, const char* path) {
    DIR* dir = opendir(path);
    if (!dir) {
        perror("Failed to open target directory");
        return EXIT_FAILURE;
    }

    printf("Entered send_dir_tree (%s), client: %d\n", path, sock);

    if (strcmp(path, SERVER_STORAGE) != 0) { // skip main directory
        header_t header;
        memset(&header, 0, sizeof header);

        header.type = MT_NEW_DIR;
        header.hsize = (uint64_t) NULL;

        char temp[1024];
        const char* path_inner = path + strlen(SERVER_STORAGE);
        snprintf(temp, sizeof temp, ".%s", path_inner[0] == '/' ? path_inner+1 : path_inner);

        printf("Inner path: %s\n", path_inner);


        strcpy(header.path, path_inner);
        int n1 = send_header(sock, &header);
        if (n1 < 0) {
            printf("Client disconnected %d during header send (NEW_DIR)\n", sock);
            return EXIT_FAILURE;
        }
    }

    struct dirent* file;
    while ((file = readdir(dir)) != NULL) {

        if (file->d_type == DT_DIR) {
            if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) {
                continue;
            }

            char* dir_path = malloc(strlen(path) + strlen(file->d_name) + 3);
            if (dir_path == NULL) {
                perror("send_dir_tree / dir_path - malloc failed");
            }

            strcpy(dir_path, path);
            strcat(dir_path, "/");
            strcat(dir_path,  file->d_name);
            send_dir_tree(sock, dir_path);
            free(dir_path);

        }
        else if (file->d_type == DT_REG) {
            char* file_path = malloc(strlen(path) + strlen(file->d_name) + 3);
            if (file_path == NULL) {
                perror("send_dir_tree / file_path - malloc failed");
            }


            strcpy(file_path, path);
            strcat(file_path, "/");
            strcat(file_path,  file->d_name);

            header_t header;
            memset(&header, 0, sizeof header);

            header.type = MT_NEW_FILE;
            header.hsize = (uint64_t) NULL;

            char temp2[1024];
            const char* file_path_inner = file_path + strlen(SERVER_STORAGE);
            snprintf(temp2, sizeof temp2, ".%s", file_path_inner[0] == '/' ? file_path_inner+1 : file_path_inner);

            strcpy(header.path, file_path_inner);

            printf("send_dir_tree -> %s (inner path: %s), client: %d\n", file_path, file_path_inner, sock);

            int n1 = send_header(sock, &header);
            if (n1 < 0) {
                printf("Client disconnected %d during header send\n", sock);
                break;
            }

            int n2 = send_file(sock, file_path_inner, ".");
            if (n2 < 0) {
                printf("Client disconnected %d during file content send\n", sock);
                break;
            }

            free(file_path);
        }
    }
    closedir(dir);
    return EXIT_SUCCESS;
}

/////////////////////////////////////////////////////////////

static int receive_file(int sock, const char *filename, char *buff, int bsize, char *path) {
    int file = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (file == -1) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    printf("read %u bytes\n", bsize);
    if (bsize > 0) {
        int nbytes_received = recv(sock, buff, bsize, MSG_WAITALL);
        if (nbytes_received <= 0) {
            perror("rcv content failed");
            return EXIT_FAILURE;
        }
        if (write(file, buff, bsize) == -1) {
            perror("Writing to file failed");
            return EXIT_FAILURE;
        }
    }
    printf("file recved\n");

    close(file);

    return EXIT_SUCCESS;
}

int receive_message(const int sock, message_t *message, const char *path_pfx) {

    header_t *header = &message->header;
    memset(header, 0, sizeof *header);

    int n = recv(sock, header, sizeof(header_t), MSG_WAITALL);
    header->hsize = be64toh(header->nsize);

    printf("Received message type: %u / size: %lu\n", header->type, header->hsize);
    if (header->type == 0) {
        printf("Received invalid message - break\n");
        return -1;
    }

    char *full_path = path_concat(path_pfx, header->path);
    printf("Received file path: <%s>,  full path: %s\n", header->path, full_path);

    if (n > 0) {
        if (header->type == MT_NEW_FILE) {
            message->content = (char *)malloc(header->hsize);
            if (message->content == NULL) {
                perror("Memory allocation failed");
                return -1;
            }
            receive_file(sock, header->path, message->content, header->hsize, full_path);
        }
        else if (header->type == MT_NEW_DIR) {
            mkdir(full_path, 0777);
        }
        else if (header->type == MT_REMOVE) {
            remove(full_path);
        }
    } else if (n == 0) {
        printf("Client disconnected %d\n", sock);
    } else if (n < 0) {
        perror("Receive header - error");
    }

    free(full_path);
    return n;
}

static char *path_concat(const char *path, const char *name) {
    int len_path = strlen(path);
    int len_name = strlen(name);
    char *new_path = malloc(len_name + len_path + 7);

    strcpy(new_path, path);
    new_path[len_path] = '/';
    strcpy(new_path + (len_path + 1), name);

    return new_path;
}
