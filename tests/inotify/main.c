#include <sys/stat.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include "file_watch.h"

int main(int argc, char *argv[]) {

    fw_state_t state;
    fw_init(&state, argv[1]);
    while (1){
        fw_handle_read(&state);
    }

    return EXIT_SUCCESS;
}
