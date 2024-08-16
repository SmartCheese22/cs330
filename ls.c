#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/syscall.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
#include<assert.h>
#include<dirent.h>
#include<string.h>
#include<time.h>
#include <pwd.h>
#include <grp.h>

#define STRINGSIZE 1024

void print_file_info(struct dirent *d){
    struct stat sb;
    if(stat(d->d_name, &sb) == -1){
        perror("stat");
        return;
    }
    printf( (S_ISDIR(sb.st_mode)) ? "d" : "-");
    printf( (sb.st_mode & S_IRUSR) ? "r" : "-");
    printf( (sb.st_mode & S_IWUSR) ? "w" : "-");
    printf( (sb.st_mode & S_IXUSR) ? "x" : "-");
    printf( (sb.st_mode & S_IRGRP) ? "r" : "-");
    printf( (sb.st_mode & S_IWGRP) ? "w" : "-");
    printf( (sb.st_mode & S_IXGRP) ? "x" : "-");
    printf( (sb.st_mode & S_IROTH) ? "r" : "-");
    printf( (sb.st_mode & S_IWOTH) ? "w" : "-");
    printf( (sb.st_mode & S_IXOTH) ? "x" : "-");

     printf(" %ld", (long) sb.st_nlink);
    printf(" %s", getpwuid(sb.st_uid)->pw_name);
    printf(" %s", getgrgid(sb.st_gid)->gr_name);

    // File size
    printf(" %lld", (long long) sb.st_size);

    // Last modification time
    char timebuf[80];
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&sb.st_mtime));
    printf(" %s", timebuf);

    // File name
    printf(" %s\n", d->d_name);
}

int main(int argc, char* argv[]){
    int long_format = 0;
    char* directory = ".";
    if(argc > 3){
        fprintf(stderr, "usage: %s [-l] [directory]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if(argc >= 2 && strcmp("-l", argv[1]) == 0){
        long_format = 1;
        if(argc > 2){
            directory = argv[2];
        }
    } else if(argc == 2){
        directory = argv[1];
    }

    DIR* dp = opendir(directory);
    assert(dp != NULL);
    struct dirent *d;

    while((d = readdir(dp)) != NULL){
        if(long_format){
            print_file_info(d);
        } else {
            printf("%s\n", d->d_name);
        }
    }
    closedir(dp);
    exit(EXIT_SUCCESS);
}
