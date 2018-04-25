#include <stdio.h>
#include <time.h>
#include <utmp.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/sysinfo.h>

#define MAXSIZE 1000

struct Users {
    long long time;
    char *name, *line, *host;
};
void alloccator(struct Users *user) {
    user->name = malloc(MAXSIZE * sizeof(char));
    user->line = malloc(MAXSIZE * sizeof(char));
    user->host = malloc(MAXSIZE * sizeof(char));
}
void makeInformationAboutUsers(struct Users *user, char* uname, char* uline, char* uhost, long long utime) {
    user->time = utime;
    strcpy(user->name, uname);
    strcpy(user->line, uline);
    strcpy(user->host, uhost);

}
void freeMemmory(struct Users *user) {
    free(user->name);
    free(user->line);
    free(user->host);
}


void printTime(long long time) {
    time %= (3600 * 24);
    long long hour = time / (60 * 60);
    long long min = (time / 60) % 60;
    long long sec = time % 60;
    printf("%02lld:%02lld:%02lld", hour, min, sec);
}

void printHeaders(int countUsers) {
    printTime(time(NULL) - timezone);
    struct sysinfo info;
    sysinfo(&info);
    if (info.uptime < 60)
        printf(" up %ld min", info.uptime / 60);
    else {
        printf(" up ");
        printTime(info.uptime);
    }
    printf(", %d user", countUsers);
    printf(",  load average: ");
    printf("%.2f %.2f %.2f",
           (double)info.loads[0] / (1 << SI_LOAD_SHIFT),
           (double)info.loads[1] / (1 << SI_LOAD_SHIFT),
           (double)info.loads[2] / (1 << SI_LOAD_SHIFT));
    printf("\n");
    printf("%-20.20s","USER");
    printf("%-20.20s","TTY");
    printf("%-20.20s","FROM");
    printf("%-20.20s","LOGIN@");
    printf("%-20.20s","IDLE");
    printf("\n");
}

void printUsersInformation(struct Users *user) {
    printf("%-20.20s", user->name);
    printf("%-20.20s", user->line);
    printf("%-20.20s", user->host);
    printTime(user->time - timezone);
    for (int i = 8; i < 20; ++i)
        printf(" ");
    printTime(time(NULL) - user->time);
    for (int i = 8; i < 20; ++i)
        printf(" ");
    printf("\n");
}

int main() {
    tzset();
    int countOfUsers = 0;
    struct utmp data;
    struct Users users[MAXSIZE];
    FILE* fin = fopen(_PATH_UTMP, "r");
    while (fread((char*)&data, sizeof(data), 1, fin) == 1) {
        if (*data.ut_line == '~')
            continue;
        if (!strcmp(data.ut_user, "LOGIN"))
            continue;
        alloccator(&users[countOfUsers]);
        makeInformationAboutUsers(&users[countOfUsers], data.ut_user, data.ut_line, data.ut_host, data.ut_time);
        countOfUsers++;
    }
    printHeaders(countOfUsers);
    for(int i = 0; i < countOfUsers; ++i) {
        printUsersInformation(&users[i]);
        freeMemmory(&users[i]);
    }
    fclose(fin);
    return 0;
}