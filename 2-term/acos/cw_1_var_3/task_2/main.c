#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>

int count_a = 0;

void counter_a(char *buffer) {
    for(unsigned i = 0; i < strlen(buffer); ++i)
        if (buffer[i] == 'a')
            ++count_a;
}

void replace_string(char* cur_str, char* finding_str) {
    size_t cur_len = strlen(cur_str);
    size_t find_len = strlen(finding_str);
    bool check = true;
    for (int i = 0 ; i < cur_len; ++i) {
        if (i + find_len <= cur_len) {
            check = true;
            for (int j = i; j < i + find_len; ++j) {
                if (cur_str[j] != finding_str[j - i]) {
                    check = false;
                    break;
                }
            }
            if (check) {
                for (int k = 0; k < count_a; ++k)
                    printf("#");
                i += find_len - 1;
            } else {
                printf("%c", cur_str[i]);
            }
        }
        else
            printf("%c", cur_str[i]);
    }
}

void process_with_file(char* name_file, char* finding_str) {
    FILE* fin = fopen(name_file, "r");
    char* buffer = NULL;
    size_t len = 0;
    char read;
    while ((read = getline(&buffer, &len, fin)) != -1) {
        replace_string(buffer, finding_str);

        free(buffer);
        buffer = NULL;
        len = 0;
    }
    fclose(fin);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Count of argv < 2");
        exit(1);
    }
    if (argv[1] == "") {
        fprintf(stderr, "Don't put a file");
        exit(1);
    }
    if (argv[2] == "") {
        fprintf(stderr, "No string to find");
        exit(1);
    }

    counter_a(argv[2]);

    process_with_file(argv[1], argv[2]);
}