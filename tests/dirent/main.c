#include <stdio.h>     /* for printf and stderr */
#include <stdlib.h>
#include <string.h>    /* for strerror */
#include <dirent.h>    /* for DIR, opendir, readdir, and dirent */

void dir_tree(char path[]){
    printf("enter %s\n", path);
    DIR* fd;
    if ((fd = opendir(path)) == NULL) {
        perror("Error : Failed to open target directory");
    }

    struct dirent* in_file;
    while ((in_file = readdir(fd))) {
        if (in_file->d_type == DT_DIR) {
            if (!(strcmp(".", in_file->d_name) == 0 || strcmp("..", in_file->d_name) == 0)){
                char *dir_path = malloc(strlen(path) + strlen(in_file->d_name) + 3);
                strcpy(dir_path, path);
                strcat(dir_path, "/");
                strcat(dir_path,  in_file->d_name);
                dir_tree(dir_path);
                free(dir_path);
            }
        }
        if (in_file->d_type == DT_REG) {
            printf("%s is file\n", in_file->d_name);
        }
    }
    closedir(fd);
}

int main(int argc, char *argv[]) {

    char* target_dir = argv[1];
    dir_tree(target_dir);
    
    return 0;
}
