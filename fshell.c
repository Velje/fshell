#define _XOPEN_SOURCE 700
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <grp.h>

#define check_error(cond)\
    do {\
        if (cond) {\
            perror("Something went wrong");\
        }\
        else {\
            puts("Success");\
        }\
    } while (0)

const uint32_t MAX_CHARACTERS = 50;

void exit_procedure(char* mem_buffer) {
    puts("Exiting...Goodbye!");
    free(mem_buffer);
    exit(EXIT_SUCCESS);
}

bool print_FileInfo(char* filePath) {

    fscanf(stdin, "%s", filePath);
    struct stat fajlInfo;
    if (stat(filePath, &fajlInfo)) {
        return true;
    }
    fprintf(stdout, "File: ");
    off_t stmode = fajlInfo.st_mode;
    switch (stmode & S_IFMT) {
        case S_IFBLK:  fprintf(stdout, "block device\n");            break;
        case S_IFCHR:  fprintf(stdout, "character device\n");        break;
        case S_IFDIR:  fprintf(stdout, "directory\n");               break;
        case S_IFIFO:  fprintf(stdout, "FIFO/pipe\n");               break;
        case S_IFLNK:  fprintf(stdout, "symlink\n");                 break;
        case S_IFREG:  fprintf(stdout, "regular file\n");            break;
        case S_IFSOCK: fprintf(stdout, "socket\n");                  break;
        default:       fprintf(stdout, "unknown?\n");                break;
    }
    struct group* grupaInfo = getgrgid(fajlInfo.st_uid);
    if (!grupaInfo) {
        return true;
    }
    fprintf(stdout, "Owner: %s\n", grupaInfo -> gr_name);
    grupaInfo = getgrgid(fajlInfo.st_gid);
    if (!grupaInfo) {
        return true;
    }
    fprintf(stdout, "Group: %s\n", grupaInfo -> gr_name);
    fprintf(stdout, "Size: %ldB\n", fajlInfo.st_size);
    fprintf(stdout,"Access: 0%lo\n", stmode & 0777);
    return false;

}

bool make_File(char* filePath) {

    fscanf(stdin, "%s", filePath);
    int fd;
    if ((fd = open(filePath, O_CREAT | O_EXCL, 0644)) < 0) {
        return true;
    }
    return close(fd);

}

bool make_Directory(char* filePath) {

    fscanf(stdin, "%s", filePath);
    return mkdir(filePath, 0755);

}

bool remove_File(char* filePath) {

    fscanf(stdin, "%s", filePath);
    return unlink(filePath);

}

bool remove_Directory(char* filePath) {

    fscanf(stdin, "%s", filePath);
    return rmdir(filePath);

}

bool cat(char* filePath) {

    fscanf(stdin, "%s", filePath);
    int fd = open(filePath, O_RDONLY);
    if (fd < 0) {
        return true;
    }
    static const uint32_t memBufSize = 1U << 16; // 64KB
    char *mem_buffer = malloc(memBufSize);
    if (mem_buffer == NULL) {
        close(fd);
        return true;
    }
    int32_t readBytes;
    while ((readBytes = read(fd, mem_buffer, memBufSize)) > 0) {
        if (write(STDOUT_FILENO, mem_buffer, readBytes) == -1) {
            free(mem_buffer);
            close(fd);
            return true;
        }
    }
    free(mem_buffer);
    if (readBytes != 0) {
        close(fd);
        return true;
    }
    return close(fd);

}

bool insert(char* filePath) {

    char* word = malloc(MAX_CHARACTERS + 1);
    if (word == NULL) {
        return true;
    }
    uint32_t position;
    fscanf(stdin, "%s", word);
    fscanf(stdin, "%u", &position);
    fscanf(stdin, "%s", filePath);
    int fd = open(filePath, O_WRONLY);
    if (fd < 0) {
        free(word);
        return true;
    }
    if (lseek(fd, position, SEEK_SET) == -1) {
        free(word);
        close(fd);
        return true;
    }
    if (write(fd, word, strlen(word)) == -1) {
        free(word);
        close(fd);
        return true;
    }
    free(word);
    return close(fd);

}

bool cp(char *filePath) {

    fscanf(stdin, "%s", filePath);
    char* fileDest = malloc(MAX_CHARACTERS);
    if (fileDest == NULL) {
        return true;
    }
    fscanf(stdin, "%s", fileDest);
    int fdSrc = open(filePath, O_RDONLY);
    if (fdSrc < 0) {
        free(fileDest);
        return true;
    }
    int fdDest = open(fileDest, O_TRUNC | O_WRONLY | O_CREAT, 0644);
    if (fdDest < 0) {
        free(fileDest);
        close(fdSrc);
        return true;
    }

    static const uint32_t memBufSize  = 1U << 16; // 64KB
    char *mem_buffer = malloc(memBufSize);
    if (mem_buffer == NULL) {
        close(fdSrc);
        close(fdDest);
        free(fileDest);
        return true;
    }
    int32_t readBytes;
    while ((readBytes = read(fdSrc, mem_buffer, MAX_CHARACTERS)) > 0) {
        if (write(fdDest, mem_buffer, readBytes) < 0) {
            free(mem_buffer);
            close(fdSrc);
            close(fdDest);
            free(fileDest);
            return false;
        }
    }

    free(mem_buffer);
    free(fileDest);
    if (readBytes != 0) {
        close(fdDest);
        close(fdSrc);
        return true;
    }
    return close(fdDest) || close(fdSrc);

}

int main() {

    char* mem_buffer = malloc(MAX_CHARACTERS + 1);
    if (!mem_buffer) {
        puts("Memory allocation failed!");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Do not use more then %u characters for file names.\n", MAX_CHARACTERS);
    fprintf(stdout, "%% ");
    while (fscanf(stdin, "%50s", mem_buffer) != EOF) {

        if (!strcmp(mem_buffer, "exit")) {
            exit_procedure(mem_buffer);
        }
        else if (!strcmp(mem_buffer, "info")) {
            puts("File information will be displayed:");
            check_error(print_FileInfo(mem_buffer));
        }
        else if (!strcmp(mem_buffer, "mkfile")) {
            puts("Making a file");
            check_error(make_File(mem_buffer));
        }
        else if (!strcmp(mem_buffer, "mkdir")) {
            puts("Making a directory");
            check_error(make_Directory(mem_buffer));
        }
        else if (!strcmp(mem_buffer, "rm")) {
            puts("Removing file");
            check_error(remove_File(mem_buffer));
        }
        else if (!strcmp(mem_buffer, "rmdir")) {
            puts("Removing directory");
            check_error(remove_Directory(mem_buffer));
        }
        else if (!strcmp(mem_buffer, "cat")) {
            puts("Reading from file:");
            check_error(cat(mem_buffer));
        }
        else if (!strcmp(mem_buffer, "insert")) {
            puts("Inserting word");
            check_error(insert(mem_buffer));
        }
        else if (!strcmp(mem_buffer, "cp")) {
            puts("Copying");
            check_error(cp(mem_buffer));
        }
        else {
            fprintf(stdout, "Unknown command: %s\n", mem_buffer);
        }
        if (getchar() == '\n') {
            fprintf(stdout, "%% ");
        }

    }
    exit_procedure(mem_buffer);
    
}
