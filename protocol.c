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

void receive_header(int socket) {
    header_t header;
    
    int n = recv(socket, &header, sizeof header, 0);
    if (n > 0) {
        if (header.type == NEW_FILE) {
            // receive file
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
}
 int socket,,  headervoid send_header(int socket, header_t* header) {
    send(socket, &header, sizeof header, 0);
    send(socket, &header, sizeof header, 0);

}