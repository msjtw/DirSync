#include <stdint.h>
#include <stdlib.h>

// FIX: BUFF_SIZE redefinition
// #define BUFF_SIZE 1024

#define MT_NEW_FILE 1
#define MT_NEW_DIR 2
#define MT_REMOVE 3

// NOTE: header.size and message.content are empty if header.type != NEW_FILE
typedef struct header {
    uint8_t type;
    uint64_t size;
    char path[256];
} header_t;


typedef struct message {
    int id;
    header_t header;
    char* content;
    int clients_sent;
    int sender_fd;
} message_t;


int send_header(int sock, header_t* header);
int send_content(int sock, const char* buf, size_t length);
int send_file(int sock, const char* filename, const char* path_pfx);
int send_dir_tree(int sock, char path[]);

int receive_message(int sock, message_t* message, char* path_pfx);
