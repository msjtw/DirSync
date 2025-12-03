#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#define EVENT_SIZE (sizeof (struct inotify_event))
#define NAME_MAX 128
#define BUFF_SIZE (32 * (EVENT_SIZE + NAME_MAX + 1))

int main(int argc, char *argv[]) {
    int fd; // inotify descriptor
    if ((fd = inotify_init1(0)) < 0){
        perror("inotify_init fail");
        exit(EXIT_FAILURE);
    }

    int wd; //  watch descriptor for the filesystem object (inode) that corresponds to path
    if ((wd = inotify_add_watch(fd, argv[1], IN_CREATE | IN_DELETE | IN_ACCESS | IN_MODIFY)) < 0){
        perror("inotify_add_watch fail");
        exit(EXIT_FAILURE);
    }

    while (1){
        char buff[BUFF_SIZE];
        memset(buff, 0, sizeof(buff));
        int len;
        if ((len = read(fd, buff, BUFF_SIZE)) < 0){
            perror("read fail");
            exit(EXIT_FAILURE);
        }

        int i = 0;
        while (i < len){
            struct inotify_event *event = (struct inotify_event *) &buff[i];
            printf("event %u\n", event->len);
            if (event->len){
                if (event->mask & IN_CREATE){
                    printf("File %s created.\n", event->name);
                    struct stat buf;          
                    stat(event->name, &buf);
                    if (buf.st_mode & S_IFDIR) {
                        printf("%s is a dir\n", event->name);
                    }
                }
                if (event->mask & IN_DELETE){
                    printf("File %s deleted.\n", event->name);
                }
                if (event->mask & IN_ACCESS){
                    printf("File %s accessed.\n", event->name);
                }
                if (event->mask & IN_MODIFY){
                    printf("File %s modified.\n", event->name);
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);

    return EXIT_SUCCESS;
}
