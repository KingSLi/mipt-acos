#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdbool.h>
#include <getopt.h>
#include <ctype.h>
#include <malloc.h>
#include <stdlib.h>


bool isA = false, isL = false;
int arguments = 0;

bool isDirectory(const char* name) {
    struct stat info;
    if (stat(name, &info) == -1) {
        return false;
    }
    return info.st_mode & __S_IFDIR;
}
bool isFile(const char* name) {
    struct stat info;
    if (stat(name, &info) == -1) {
        return false;
    }
    return info.st_mode & __S_IFREG;
}

char* create_normal_path(char * path) {
    if (path[0] == '/' || path[0] == '.')
        return path;
    if (path[0] != '~' && path[0] != '.' && path[0] != '\\') {
        char *new_path = malloc(sizeof(path) + 2);
        strcpy(new_path, "./");
        strncat(new_path, path, strlen(path));
        //fprintf(stderr, "%s\n", new_path);
        return new_path;
    }
}

void print_information_about_file(char *name_dir) {
    struct stat fileStat;
    stat(name_dir, &fileStat);
    if (!isA && !isL) {
        if (name_dir[0] != '.')
            printf("%s ", name_dir);
    } else if (isA && !isL) {
        printf("%s ", name_dir);
    } else if (isL) {
        if (!isA && name_dir[0] == '.')
            return;
        time_t curT = fileStat.st_mtime;
        printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
        printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
        printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
        printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
        printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
        printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
        printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
        printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
        printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
        printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");
        printf(" %ld", fileStat.st_nlink);
        printf(" %s", getpwuid(fileStat.st_gid)->pw_name);
        printf(" %s", getgrgid(fileStat.st_gid)->gr_name);
        printf(" %-10ld ", fileStat.st_size);

        char *buf = ctime(&curT);
        for (int i = 4; i < strlen(buf) - 9; i++) {
            printf("%c", buf[i]);
        }
        printf(" %s", name_dir);
        printf("\n");
    }
}

void ls(char * input_name) {
    bool isFil = isFile(input_name);
    bool isDir = isDirectory(input_name);
    if (!isDir && !isFil) {
        printf("Can't find file/directory: %s\n\n", input_name);
        return;
    }
    if (arguments > 1 && isDir)
        printf("%s:\n", input_name);
    struct dirent* filetat;
    DIR* myDir;
    if (!isDir) {
        print_information_about_file(input_name);
        printf("\n");
        return;
    }
    char *dir_name = create_normal_path(input_name);
    if ((myDir = opendir(dir_name)) == NULL) {
        fprintf(stderr, "Can't open the dir: %s!\n", dir_name);
        exit(1);
    }
    while ((filetat = readdir(myDir)) != NULL || !isDir) {
        print_information_about_file(filetat->d_name);
    }
    closedir(myDir);
    free(filetat);
    printf("\n");
}

int main(int argc, char** argv){
    char c;
    while ((c = getopt(argc, argv, "la")) != -1) {
        if (c == 'l') {
            isL = true;
        } else if (c == 'a') {
            isA = true;
        } else {
            fprintf(stderr, "Unresolved arguments!\n");
            exit(1);
        }
    }

    for (size_t i = 1; i < argc; ++i) {
        if (argv[i][0] != '-')
            ++arguments;
    }
    if (!arguments) {
        ls(".");
    } else {
        for (size_t i = 1; i < argc; ++i) {
            if (argv[i][0] == '-')
                continue;
            ls(argv[i]);
        }
    }
    return 0;
}




