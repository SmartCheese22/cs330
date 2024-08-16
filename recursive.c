#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/syscall.h>
#include<sys/types.h>
#include<fcntl.h>
#include<dirent.h>
#include<string.h>
#include<sys/stat.h>

void listdir(const char* name, int depth){
    DIR *dir;
    struct dirent *entry;
    if(!(dir = opendir(name))){
        return ;
    }

    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }
        for(int i = 0; i < depth; i++){
            printf("-");
        }

        printf(">%s\n", entry->d_name);

        struct stat statbuf;
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);

        if(stat(path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)){
            listdir(path, depth + 1);
        }
    }

    closedir(dir);
}

int main(int argc, char* argv[]){
    const char* filename = (argc > 1) ? argv[1] : ".";
    listdir(filename, 0);
    return 0;
}
