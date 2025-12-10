#include <stdint.h>

enum MSG_TYPE{
    NEW_FILE,
    NEW_DIR,
    REMOVE,
};

typedef struct header {
    enum MSG_TYPE type;
    uint64_t size;
    char path[256];
} header_t;

void receive_header();

void send_header();
