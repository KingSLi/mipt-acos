#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

struct shared {
    int n;
    int *table;
    pthread_mutex_t *table_lock;
};

struct ret {
    int luck;
    pid_t pid;
};

void *play(void *param_raw) {
    struct shared *param = (struct shared*)param_raw;
    int luck = 0;
    pid_t pid = getpid();
    for (int shot = 0; shot < 10; ++shot) {
        int i = rand() % param->n;
        int j = rand() % param->n;

        pthread_mutex_lock(&param->table_lock[i * param->n + j]);
        if (!param->table[i * param->n + j]) {
            ++luck;
            param->table[i * param->n + j] = pid;
        }
        pthread_mutex_unlock(&param->table_lock[i * param->n + j]);

        sleep(1 + rand() % 60);
    }

    struct ret *ret = malloc(sizeof(struct ret));
    ret->luck = luck;
    ret->pid = pid;
    return (void*)ret;
}

int main(int argc, char **argv) {
    srand(time(NULL));

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <N> <P>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int p = atoi(argv[2]);

    int *table = malloc(n * n * sizeof(int));
    if (!table) {
        perror(argv[0]);
        return 2;
    }

    pthread_mutex_t *table_lock = malloc(n * n * sizeof(pthread_mutex_t));
    if (!table_lock) {
        perror(argv[0]);
        return 2;
    }

    memset(table, 0, n * n * sizeof(int));
    for (int i = 0; i < n * n; ++i) {
        pthread_mutex_t tmp = PTHREAD_MUTEX_INITIALIZER;
        table_lock[i] = tmp;
    }

    struct shared data;
    data.n = n;
    data.table = table;
    data.table_lock = table_lock;

    for (int round = 0; round < 10; ++round) {
        printf("round %d\n", round);
        pthread_t thr[10];
        for (int i = 0; i < 10; ++i) {
            int code = pthread_create(&thr[i], NULL, play, (void*)&data);
            if (code) {
                errno = code;
                perror(argv[0]);
                return 3;
            }
        }
        for (int i = 0; i < 10; ++i) {
            void *ret_raw;
            pthread_join(thr[i], &ret_raw);

            struct ret *ret = (struct ret*)ret_raw;

            printf("thread %d: luck %d\n", ret->pid, ret->luck);

            free(ret_raw);
        }
        printf("\n");
    }

    return 0;
}