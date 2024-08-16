#include<stdio.h>
#include<stdlib.h>
#include<sys/syscall.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<time.h>
#include<assert.h>

void mytail(const char* filename, int n){
    int fd = open(filename, O_RDONLY);
    if(fd < 0){
        perror("open");
        return;
    }
    struct stat sb;
    fstat(fd, &sb);
    off_t filesize = sb.st_size;

    off_t pos = filesize;
    int linecount = 0;
    char buffer[1];

    while(pos > 0 && linecount < n){
        lseek(fd, --pos, SEEK_SET);
        read(fd, buffer, 1);
        if(buffer[0] == '\n'){
            linecount++;
        }
    }

    lseek(fd, pos + 1, SEEK_SET);
    while(read(fd, buffer, 1) > 0){
        write(STDOUT_FILENO, buffer, 1);
    }
    close(fd);
}

int main(int argc, char* argv[]){
    if(argc != 3){
        fprintf(stderr, "usage:%s -n <number_of_lines> <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int n = atoi(argv[1]);
    const char* filename = argv[2];

    if(n <= 0){
        fprintf(stderr, "Invalid number of lines\n");
        exit(EXIT_FAILURE);
    }
    mytail(filename, n);
    return 0;
}
