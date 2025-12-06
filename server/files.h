// files.h

#ifndef FILES_H
#define FILES_H

int send_file(int client_socket, const char* filename);
int send_file_list(int client_socket, const char* folder_path);
int send_directory(int client_socket, const char* path);
int receive_file(int server_socket, const char* filename);
int receive_file_list(int server_socket);

#endif