#include <stdint.h>
#include <stdlib.h>

// FIX: BUFF_SIZE redefinition
// #define BUFF_SIZE 1024

#define MT_NEW_FILE 1
#define MT_NEW_DIR 2
#define MT_REMOVE 3

#define SERVER_STORAGE "./storage/"

// NOTE: header.size and message.content are empty if header.type != NEW_FILE
typedef struct header {
    uint8_t type;
    uint64_t nsize; // network ordered size
    uint64_t hsize; // host ordered size
    char path[256];
} header_t;

typedef struct message {
    int id;
    header_t header;
    char *content;
    int clients_sent;
    int sender_fd;
} message_t;

int send_header(const int sock, const header_t *header);
int send_content(const int sock, const message_t *msg);
int send_file(const int sock, const header_t *header, const char *path_pfx);
int send_dir_tree(const int sock, const char *path);

int receive_message(const int sock, message_t *message, const char *path_pfx);
