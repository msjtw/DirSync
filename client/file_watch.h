#include <stdlib.h>
#include <sys/inotify.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define NAME_MAX 128
#define BUFF_SIZE (32 * (EVENT_SIZE + NAME_MAX + 1))

#define IN_FLAGS IN_CREATE | IN_DELETE | IN_MODIFY

typedef struct fw_state {
    int fd;
    size_t size;
    char **wd;
} fw_state_t;

void fw_init(fw_state_t *state, char *path);

void fw_close(fw_state_t *state);

void fw_handle_read(fw_state_t *state);

static void add_fd(fw_state_t *state, char *path);

static void rem_fd(fw_state_t *state, int wd);

static char *path_concat(const char *path, const char *name);
