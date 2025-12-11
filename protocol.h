#include <stdint.h>

enum MSG_TYPE{
    NEW_FILE,
    NEW_DIR,
    REMOVE,
};

// NOTE header.size and header.path are empty if header.type != NEW_FILE
typedef struct header {
    enum MSG_TYPE type;
    uint64_t size;
    char path[256];
} header_t;


int send_header(int sock, header_t* header);
int send_content(int sock, const void* buf, size_t length);
int send_file(int sock, const char* filename);

int receive_header(int sock);
int receive_file(int sock, const char* filename);
