#include "../protocol.h"

int connection_init(const char* port, const char* address);

message_t connection_read(int fd);

void connection_write(int fd, message_t *message);

