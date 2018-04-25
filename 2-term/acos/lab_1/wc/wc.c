#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

#define total "итого"

long long bytes = 0, words = 0, lines = 0;
long long local_bytes = 0, local_words = 0, local_lines = 0;
bool isL = false, isW = false, isC = false;

void parse_opt(int argc, char* args[]) {
    char c;
    while ((c = getopt(argc, args, "lwc")) != -1) {
        switch (c) {
            case 'l' : isL = true; break;
            case 'w' : isW = true; break;
            case 'c' : isC = true; break;
        }
    }
    if (!(isW || isL || isC))
        isC = isL = isW = true;
}

void reloadVar() {
    local_words = local_bytes = local_lines = 0;
}

void readFileByRead(char* name) {
    int fn = open(name, O_RDONLY);
    ssize_t bytesread;
    char last = 'a';
    do {
        char t = 0;
        bytesread = read(fn, &t, 1);
        if (bytesread != 0)
            ++local_bytes;
        if (t == '\n') {
            ++local_lines;
            if (last != ' ' && last != '\n')
                ++local_words;
        } else if (t == '\0') {
            if (last != ' ' && last != '\n')
                ++local_words;
        } else if (t != ' ' && last == ' ') {
            ++local_words;
        }
        last = t;
    }
    while (bytesread != 0);

    bytes += local_bytes;
    lines += local_lines;
    words += local_words;
}

void printInformation(bool isCur, char *name) {
    if (isL)
        printf("%-10lld", isCur ? local_lines : lines);
    if (isW)
        printf("%-10lld", isCur ? local_words : words);
    if (isC)
        printf("%-10lld", isCur ? local_bytes : bytes);
    printf("%s", name);
    printf("\n");
}

int main(int argc, char** argv) {
    parse_opt(argc, argv);
    int countFiles = 0;
    for (int i = 1 ;i < argc; ++i) {
        if (argv[i][0] == '-')
            continue;
        reloadVar();
        //readCurrentFile(argv[i]);
        readFileByRead(argv[i]);
        printInformation(true, argv[i]);
        countFiles++;
    }
    if (countFiles > 1)
        printInformation(false, total);
    return 0;
}