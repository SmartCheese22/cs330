#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

#define BUF_SIZE 4096

// Function to get the size of a file or directory (including all its contents)
unsigned long get_size(const char *path) {
    struct stat sb;
    if (lstat(path, &sb) == -1) {
        perror("lstat");
        return 0;
    }

    unsigned long total_size = 0;

    // If it's a symbolic link, resolve it and get the size of the target
    if (S_ISLNK(sb.st_mode)) {
        char target[BUF_SIZE];
        ssize_t len = readlink(path, target, BUF_SIZE);
        if (len == -1) {
            perror("readlink");
            return 0;
        }
        target[len] = '\0';
        return get_size(target);
    }

    // If it's a regular file, return its size
    if (S_ISREG(sb.st_mode)) {
        return sb.st_size;
    }

    // If it's a directory, calculate the size recursively
    if (S_ISDIR(sb.st_mode)) {
        total_size += sb.st_size;
        DIR *dir = opendir(path);
        if (dir == NULL) {
            perror("opendir");
            return 0;
        }
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char new_path[BUF_SIZE];
            snprintf(new_path, BUF_SIZE, "%s/%s", path, entry->d_name);
            total_size += get_size(new_path);
        }
        closedir(dir);
    }

    return total_size;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *root_dir = argv[1];
    DIR *dir = opendir(root_dir);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    int num_subdirs = 0;

    // First count the number of subdirectories
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            num_subdirs++;
        }
    }
    closedir(dir);

    int pipes[num_subdirs][2];
    pid_t pids[num_subdirs];
    int i = 0;

    // Reopen the directory to process subdirectories
    dir = opendir(root_dir);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            pids[i] = fork();
            if (pids[i] == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (pids[i] == 0) {
                // Child process
                close(pipes[i][0]);

                char sub_dir_path[BUF_SIZE];
                snprintf(sub_dir_path, BUF_SIZE, "%s/%s", root_dir, entry->d_name);

                unsigned long sub_dir_size = get_size(sub_dir_path);
                write(pipes[i][1], &sub_dir_size, sizeof(sub_dir_size));
                close(pipes[i][1]);
                exit(EXIT_SUCCESS);
            } else {
                // Parent process
                close(pipes[i][1]);
                i++;
            }
        }
    }
    closedir(dir);

    unsigned long total_size = get_size(root_dir);

    for (int j = 0; j < num_subdirs; j++) {
        unsigned long subdir_size;
        read(pipes[j][0], &subdir_size, sizeof(subdir_size));
        total_size += subdir_size;
        close(pipes[j][0]);
        waitpid(pids[j], NULL, 0);
    }

    printf("%lu\n", total_size);

    return 0;
}
