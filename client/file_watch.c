#include "file_watch.h"
#include "../protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>

static void add_fd(fw_state_t *state, char *name, char *pfx_path);
static void rem_fd(fw_state_t *state, int wd);
static char *path_concat(const char *path, const char *name);

int fw_init(fw_state_t *state, char *path) {
    int fd; // inotify descriptor
    if ((fd = inotify_init1(0)) < 0) {
        perror("inotify_init fail");
        exit(EXIT_FAILURE);
    }

    state->fd = fd;
    state->size = 1;
    state->wd = malloc(state->size * sizeof(char *));

    int wd; //  watch descriptor for the filesystem object (inode) that
            //  corresponds to path
    if ((wd = inotify_add_watch(state->fd, path, IN_FLAGS)) < 0) {
        perror("inotify_add_watch fail");
        exit(EXIT_FAILURE);
    }
    state->wd[wd] = malloc(strlen("."));
    strcpy(state->wd[wd], ".");

    int pfds[2];
    pipe(pfds);

    state->msg_fd = pfds[1];

    return pfds[0];
}

void fw_close(fw_state_t *state) {
    for (int wd = 0; wd < state->size; wd++) {
        if (state->wd[wd] != NULL) {
            inotify_rm_watch(state->fd, wd);
        }
        free(state->wd[wd]);
    }
    free(state->wd);
    close(state->fd);
}

void fw_handle_read(fw_state_t *state, char *path_pfx) {
    int fd = state->fd;
    char buff[BUFF_SIZE];
    memset(buff, 0, sizeof(buff));
    int len;
    if ((len = read(fd, buff, BUFF_SIZE)) < 0) {
        perror("read fail");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    while (i < len) {
        struct inotify_event *event = (struct inotify_event *)&buff[i];
        if (event->len) {
            char *file_path = path_concat(state->wd[event->wd], event->name);
            char *pfx_path = path_concat(path_pfx, file_path);
            printf("file event %b path:>%s< pfx_path:>%s<\n", event->mask, file_path, pfx_path);

            header_t msg;
            memset(&msg, 0, sizeof msg);
            if (event->mask & (IN_CREATE | IN_MODIFY | IN_CLOSE_WRITE)) {
                // file modified
                printf("File %s changed.\n", pfx_path);

                struct stat filestat;
                stat(pfx_path, &filestat);

                if (filestat.st_mode & S_IFDIR) {
                    // dir creat
                    add_fd(state, file_path, pfx_path);
                    msg.type = MT_NEW_DIR;
                } else {
                    // file creat
                    msg.type = MT_NEW_FILE;
                    msg.hsize = filestat.st_size;
                    msg.nsize = htobe64(msg.hsize);
                }
                strcpy(msg.path, file_path);

                write(state->msg_fd, &msg, sizeof msg);
            }
            if (event->mask & (IN_DELETE | IN_MOVED_FROM)) {
                // file deleted
                printf("File %s deleted.\n", pfx_path);

                struct stat filestat;
                stat(pfx_path, &filestat);

                if (filestat.st_mode & S_IFDIR) {
                    printf("%s is a dir\n", pfx_path);
                    rem_fd(state, event->wd);
                }

                msg.type = MT_REMOVE;
                strcpy(msg.path, file_path);

                write(state->msg_fd, &msg, sizeof msg);
            }

            free(file_path);
            free(pfx_path);
        }
        i += EVENT_SIZE + event->len;
    }
}

static void add_fd(fw_state_t *state, char *name, char *pfx_path) {
    int wd; //  watch descriptor for the filesystem object (inode) that
            //  corresponds to path
    printf("adding dir: >%s<\n", name);
    if ((wd = inotify_add_watch(state->fd, pfx_path, IN_FLAGS)) < 0) {
        perror("inotify_add_watch fail");
        exit(EXIT_FAILURE);
    }

    if (state->size < wd) {
        state->size <<= 1;
        state->wd = realloc(state->wd, state->size * sizeof(char *));
    }
    state->wd[wd] = malloc(strlen(name));
    strcpy(state->wd[wd], name);
}

static void rem_fd(fw_state_t *state, int wd) {
    inotify_rm_watch(state->fd, wd);
    free(state->wd[wd]);
    state->wd[wd] = NULL;
}

static char *path_concat(const char *path, const char *name) {
    int len_path = strlen(path);
    int len_name = strlen(name);
    char *new_path = malloc(len_name + len_path + 7);

    strcpy(new_path, path);
    new_path[len_path] = '/';
    strcpy(new_path + (len_path + 1), name);

    return new_path;
}
