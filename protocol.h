#include <stdint.h>
#include <stdlib.h>

// FIX: BUFF_SIZE redefinition
// #define BUFF_SIZE 1024

enum MSG_TYPE{
    NEW_FILE,
    NEW_DIR,
    REMOVE,
};

// NOTE: header.size and message.content are empty if header.type != NEW_FILE
typedef struct header {
    enum MSG_TYPE type;
    uint64_t size;
    char path[256];
} header_t;


typedef struct message {
    int id;
    header_t header;
    char* content;
    int clients_sent;
} message_t;


int send_header(int sock, header_t* header);
int send_content(int sock, const void* buf, size_t length);
int send_file(int sock, const char* filename);

int receive_message(int sock, message_t* message);