#include "file_watch.h"
#include "../protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>

static void add_fd(fw_state_t *state, char *name);
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
    state->wd[wd] = path;

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

void fw_handle_read(fw_state_t *state) {
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
            printf("event wd %u\n", event->wd);
            char *file_path = path_concat(state->wd[event->wd], event->name);
            header_t msg;
            memset(&msg, 0, sizeof msg);
            if (event->mask & (IN_CREATE | IN_MODIFY)) {
                // file modified
                // printf("File %s created.\n", file_path);

                struct stat filestat;
                stat(file_path, &filestat);

                if (filestat.st_mode & S_IFDIR) {
                    // dir creat
                    add_fd(state, file_path);
                    msg.type = NEW_DIR;
                } else {
                    // file creat
                    msg.type = NEW_FILE;
                    msg.size = filestat.st_size;
                }
                strcpy(msg.path, file_path);
            }
            if (event->mask & IN_DELETE) {
                // printf("File %s deleted.\n", event->name);

                msg.type = REMOVE;
                strcpy(msg.path, file_path);

                // struct stat buf;
                // stat(event->name, &buf);
                // if (buf.st_mode & S_IFDIR) {
                //     printf("%s is a dir\n", event->name);
                //     rem_fd(state, event->wd);
                // }
            }

            write(state->msg_fd, &msg, sizeof msg);

            free(file_path);
            file_path = NULL;
        }
        i += EVENT_SIZE + event->len;
    }
}

static void add_fd(fw_state_t *state, char *name) {
    int wd; //  watch descriptor for the filesystem object (inode) that
            //  corresponds to path
    if ((wd = inotify_add_watch(state->fd, name, IN_FLAGS)) < 0) {
        perror("inotify_add_watch fail");
        exit(EXIT_FAILURE);
    }

    if (state->size < wd) {
        state->size <<= 1;
        state->wd = realloc(state->wd, state->size * sizeof(char *));
    }
    state->wd[wd] = name;
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
