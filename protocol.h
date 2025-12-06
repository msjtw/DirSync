#include <stdint.h>

typedef struct mesage {
    uint8_t type;
    uint64_t name_length;
    uint64_t size;
    char *name;
    char *file_content;
} message_t;
