#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <unistd.h>

#define BUFF_SIZE 73333
#define NAME_LEN 50

pthread_t pthreadReceive;
int PORT = 1337;

int isDigit(char c) {
    return c >= '0' && c <= '9';
}

void writeNumberInBytes(char *str, size_t x) {
    for (int i = 3; i >= 0; i--) {
        str[i] = (char)(x % 256);
        x /= 256;
    }
}

size_t readNumberFromBytes(char *string) {
    size_t res = 0;
    for (int i = 0; i < 4; ++i) {
        res = res * 256 + (string[i] + 256) % 256;
    }
    return res;
}

size_t readNumberFromDigits(char *str) {
    int i = 0;
    while (str[i] == ' ')
        ++i;
    size_t res = 0;
    for (i; '0' <= str[i] && str[i] <= '9'; ++i) {
        res = res * 10 + str[i] - '0';
    }
    return res;
}

ssize_t receiveBytes(int sock, char* buffer, size_t count) {
    size_t bytesRead = 0;
    while (bytesRead != count) {
        ssize_t cntRead = recv(sock, buffer + bytesRead, count - bytesRead, 0);
        if (cntRead <= 0) {
            return cntRead;
        }
        bytesRead += cntRead;
    }
    return count;
}

ssize_t receiveCommand(int sock, char* buffer) {
    ssize_t status = 0;
    if ((status = receiveBytes(sock, buffer, 5)) <= 0) {
        return status;
    }
    size_t countBytes = readNumberFromBytes(buffer + 1);
    if (countBytes == 0)
        return 5;
    if ((status = receiveBytes(sock, buffer + 5, countBytes)) <= 0) {
        return status;
    }
    return countBytes + 5;
}

size_t getCountLinesFromMessage(char *mess) {
    size_t bytesCount = readNumberFromBytes(mess + 1);
    mess += 5;
    size_t curBytesRead = 0;
    size_t countStrings = 0;
    while (curBytesRead != bytesCount) {
        size_t curLenString = readNumberFromBytes(mess + curBytesRead);
        curBytesRead += curLenString + 4;
        countStrings++;
    }
    return countStrings;
}

size_t getLenLineFromMessage(char *mess, size_t ind) {
    mess += 5;
    size_t len = 0;
    for (int i = 0; i <= ind; ++i) {
        len = readNumberFromBytes(mess);
        mess += len + 4;
    }
    return len;
}

size_t getLenMess(char *mess) {
    size_t lenBody = readNumberFromBytes(mess + 1);
    return lenBody + 5;
}

char* getStrFromMess(char *mess, size_t ind) {
    mess += 5;
    size_t len = 0;
    for (int i = 0; i < ind; ++i) {
        len = readNumberFromBytes(mess);
        mess += len + 4;
    }
    return mess + 4;
}

void setType(char *mess, char c) {
    mess[0] = c;
}

void makeMess(char* mess) {
    writeNumberInBytes(mess + 1, 0);
}

void makeMessByStr(char* mess, char* str, size_t countBytes) {
    writeNumberInBytes(mess + 1, countBytes + 4);
    writeNumberInBytes(mess + 5, countBytes);
    for (int i = 0; i < countBytes; ++i) {
        mess[i + 9] = str[i];
    }
}

void addStr(char *mess, char* str, size_t countBytes) {
    size_t oldLen = readNumberFromBytes(mess + 1);
    writeNumberInBytes(mess + 1, oldLen + countBytes + 4);
    writeNumberInBytes(mess + oldLen + 5, countBytes);
    for (int i = 0; i < countBytes; ++i) {
        mess[i + oldLen + 9] = str[i];
    }
}

void printStr(char *str, size_t len) {
    for (int i = 0; i < len; ++i) {
        printf("%c", str[i]);
    }
}

void printTime(time_t sec) {
    struct tm* timeInfo = localtime(&sec);
    printf("%d:%d%d ", timeInfo->tm_hour, (timeInfo->tm_min) / 10, timeInfo->tm_min % 10);
}

size_t makeLoginMess(char *msg, char *login, char *password) {
    setType(msg, 'i');
    makeMessByStr(msg, login, strlen(login));
    addStr(msg, password, strlen(password));
    return getLenMess(msg);
}

void parseKickMessage(char* line, size_t* id, char* reason) {
    *id = readNumberFromDigits(line + 5);
    size_t i = 0;
    while (!isDigit(line[i])) {
        ++i;
    }
    while (isDigit(line[i])) {
        ++i;
    }
    while (line[i] == ' ') {
        ++i;
    }
    strcpy(reason, line + i);
}

void printStatus(size_t status) {
    if (status == 0) {
        printf("OK\n");
    } else if (status == 3) {
        printf("Authentication error\n");
    } else if (status == 2) {
        printf("Not login user\n");
    } else if (status == 6) {
        printf("Invalid message\n");
    } else if (status == 5) {
        printf("Access Error\n");
    } else {
        printf("Undefined status: %d\n", (int)status);
    }
}

void makeLogin(int sockId) {
    printf("Enter login and password:\n");
    printf("your login: ");

    size_t tmpSize;
    char *userName = malloc(NAME_LEN * sizeof(char));
    getline(&userName, &tmpSize, stdin);
    userName[strlen(userName) - 1] = 0;
    printf("your password: ");
    char *userPassword = malloc(NAME_LEN * sizeof(char));
    getline(&userPassword, &tmpSize, stdin);
    userPassword[strlen(userPassword) - 1] = 0;

    char *sendMsgBuff = malloc(sizeof(char) * BUFF_SIZE);
    size_t countBytes = makeLoginMess(sendMsgBuff, userName, userPassword);
    if (send(sockId, sendMsgBuff, countBytes, 0) != countBytes) {
        printf("couldn't send login message\n");
    }
}

void makeMessageM(char* mess) {
    char* timeStr = getStrFromMess(mess, 0);
    size_t sec = readNumberFromBytes(timeStr);
    printTime(sec);
    printStr(getStrFromMess(mess, 1), getLenLineFromMessage(mess, 1));
    printf("\n");
}

void makeMessageKick(char* mess) {
    printf("I was kicked because: ");
    printStr(getStrFromMess(mess, 0), getLenLineFromMessage(mess, 0));
    printf("\n");
}

void makeMessageHistory(char* mess) {
    size_t messCount = getCountLinesFromMessage(mess) / 3;
    printf("MessagesCount: %ld\n", messCount);
    for (size_t i = 0; i < messCount; ++i) {
        char *timestamp = getStrFromMess(mess, 3 * i);
        printf("time:");
        size_t sec = readNumberFromBytes(timestamp);
        printTime(sec);
        char *login = getStrFromMess(mess, 3 * i + 1);
        char *body = getStrFromMess(mess, 3 * i + 2);
        printStr(login, getLenLineFromMessage(mess, 3 * i + 1));
        printf(" ");
        printStr(body, getLenLineFromMessage(mess, 3 * i + 2));
        printf("\n");
    }
}

void makeMessageList(char* mess) {
    size_t usersCount = getCountLinesFromMessage(mess) / 2;
    printf("UsersCount:%ld\n", usersCount);
    for (size_t i = 0; i < usersCount; ++i) {
        size_t userId = readNumberFromBytes(getStrFromMess(mess, 2 * i));
        char *login = getStrFromMess(mess, 2 * i + 1);
        printf("%ld ", userId);
        for (int j = 0; j < getLenLineFromMessage(mess, 2 * i + 1); ++j) {
            printf("%c", login[j]);
        }
        printf("\n");
    }
}

void makeMessageRegular(char * mess) {
    char *timeStr = getStrFromMess(mess, 0);
    time_t sec = readNumberFromBytes(timeStr);
    printTime(sec);
    printf("user: %s ", getStrFromMess(mess, 1));
    printf("mess: "); printStr(getStrFromMess(mess, 2),
                                     getLenLineFromMessage(mess, 2));
    printf("\n");
}

void* listenServer(void * data) {
    int serverSock = *(int*)data;
    char buffer[BUFF_SIZE];
    while (1) {
        ssize_t count = receiveCommand(serverSock, buffer);
        if (count <= 0) {
            fprintf(stderr, "Прервано соединение с сервером\n");
            close(serverSock);
            _exit(-1);
            return NULL;
        }
        if (buffer[0] == 's') {
            printStatus(readNumberFromBytes(buffer + 9));
        }
        if (buffer[0] == 'r') {
            makeMessageRegular(buffer);
        }
        if (buffer[0] == 'l') {
            makeMessageList(buffer);
        }
        if (buffer[0] == 'h') {
            makeMessageHistory(buffer);
        }
        if (buffer[0] == 'm') {
            makeMessageM(buffer);
        }
        if (buffer[0] == 'k') {
            makeMessageKick(buffer);
        }
    }
}

void sendMessH(int sock, size_t count) {
    char buffer[BUFF_SIZE];
    setType(buffer, 'h');
    char cntString[4];
    writeNumberInBytes(cntString, count);
    makeMessByStr(buffer, cntString, 4);
    ssize_t cntSend = send(sock, buffer, getLenMess(buffer), 0);
    if (cntSend != getLenMess(buffer)) {
        printf("Can't send history message\n");
    }
}

void sendMessK(int sock, size_t id, char *reason) {
    char buffer[BUFF_SIZE];
    setType(buffer, 'k');
    char idString[4];
    writeNumberInBytes(idString, id);
    makeMessByStr(buffer, idString, 4);
    addStr(buffer, reason, strlen(reason));
    ssize_t cntSend = send(sock, buffer, getLenMess(buffer), 0);
    if (cntSend != getLenMess(buffer)) {
        printf("Can't send kick message\n");
    }
}

void makeCommand(int sock, char *line) {
    char buff[100];
    if (!strcmp(line, "/list")) {
        makeMess(buff);
        setType(buff, 'l');
        send(sock, buff, 5, 0);
    } else if (!strcmp(line, "/logout")) {
        makeMess(buff);
        setType(buff, 'o');
        send(sock, buff, 5, 0);
    } else if (!strcmp(line, "/login")) {
        makeLogin(sock);
    }  else if (!strncmp(line, "/history", 8)) {
        size_t count = readNumberFromDigits(line + 8);
        sendMessH(sock, count);
    } else if (!strncmp(line, "/kick", 5)) {
        size_t userKickId;
        char reason[BUFF_SIZE];
        parseKickMessage(line, &userKickId, reason);
        sendMessK(sock, userKickId, reason);
    } else if (line[0] != '/') {
        char buffer[BUFF_SIZE];
        setType(buffer, 'r');
        makeMessByStr(buffer, line, strlen(line));
        send(sock, buffer, getLenMess(buffer), 0);
    } else {
        printf("Unknown command\n");
    }
}

int main(int argc, char** argv) {
    int option;
    char ipAddress[20];
    strcpy(ipAddress, "127.0.0.1");
    while ((option = getopt(argc, argv, "i:p:")) != -1) {
        switch (option) {
            case 'i':
                strcpy(ipAddress, optarg);
                break;
            case 'p':
                PORT = atoi(optarg);
                break;
            default:
                break;
        }
    }
    int clientSock;
    if ((clientSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        fprintf(stderr, "Socket opening failed\n");
        return 1;
    }
    struct sockaddr_in port;
    port.sin_family = AF_INET;
    port.sin_port = htons((uint16_t)PORT);
    port.sin_addr.s_addr = inet_addr(ipAddress);
    if (connect(clientSock, (struct sockaddr *) &port, sizeof(port)) < 0) {
        fprintf(stderr, "Connect to server failed\n");
        return 2;
    }
    printf("Welcome to chat\n");
    printf("Available commands: /login /logout /history /list /kick\n");
    pthread_create(&pthreadReceive, NULL, listenServer, (void *)&clientSock);
    ssize_t read;
    size_t len = 0;
    char* line = NULL;
    while ((read = getline(&line, &len, stdin)) != -1) {
        line[read - 1] = 0;
        makeCommand(clientSock, line);
    }
    return 0;
}