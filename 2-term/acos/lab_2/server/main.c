#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <zconf.h>
#include <stdbool.h>
#include <pthread.h>
#include <memory.h>

#include "declarations.h"
#include "processing.h"

int PORT = 1337;

pthread_t threads[MAX_CONNECTIONS];
pthread_mutex_t data_mutex;
int count_of_connections_users = 0;
int messagesInHistoryCount = 0;


int sendMessageToActiveUsers(char* mess) {
    size_t len = getLenMessage(mess);
    int ok = 0;
    pthread_mutex_lock(&data_mutex);
    for (int i = 0; i < count_of_connections_users; ++i) {
        if (users[i].is_active) {
            ssize_t bytesSend = send(users[i].socket_id, mess, len, 0);
            if (bytesSend != len) {
                fprintf(stderr, "Can't send message to %s\n", users[i].login);
                ok = -1;
            }
        }
    }
    pthread_mutex_unlock(&data_mutex);
    return ok;
}

int addUser(int sock, char* login, char* password, int* isNew, int userId) {
    int isNewLogin = 1;
    int userFind = -1;
    size_t userFindInd = 0;
    pthread_mutex_lock(&data_mutex);
    for (size_t i = 0; i < count_of_connections_users; ++i) {
        if (0 == strcmp(login, users[i].login)) {
            if (strcmp(password, users[i].password) != 0) {
                pthread_mutex_unlock(&data_mutex);
                return -3;
            } else if (!users[i].is_active) {
                users[i].is_active = true;
                users[i].socket_id = sock;
                *isNew = -1;
            }
            userFind = 1;
        }
    }
    if (userFind == 1) {
        return userId;
    }
    *isNew = 1;
    strcpy(users[userId].login, login);
    strcpy(users[userId].password, password);
    users[userId].is_active = true;
    users[userId].socket_id = sock;
    users[userId].is_loginned = true;
    pthread_mutex_unlock(&data_mutex);
    return userId;
}


void addMessageToHistory(char *mess) {
    memcpy(history[messagesInHistoryCount++], mess,
           getLenMessage(mess));
}

int processLogin(char *msg, int *userId, int sock) {
    char login[NAME_LEN];
    char password[NAME_LEN];
    if (getCountStringsMessage(msg) == 2) {
        size_t lenLogin = getLenOfStringByIndex(msg, 0);
        size_t lenPassword = getLenOfStringByIndex(msg, 1);
        if (lenLogin < 2 || lenPassword < 2 || lenLogin > 31 || lenPassword > 31) {
            sendStatusMessage(sock, 3);
            return -3;
        }
        strncpy(login, getStringByIndex(msg, 0), lenLogin);
        strncpy(password, getStringByIndex(msg, 1), lenPassword);
        login[lenLogin] = 0;
        password[lenPassword] = 0;
        int isNew = 1;
        if ((addUser(sock, login, password, &isNew, *userId)) >= 0) {
            sendStatusMessage(sock, 0);
            if (isNew) {
                char buffer[SMALL_BUFF];
                createNotificationLoginMessage(buffer, *userId, login);
                sendMessageToActiveUsers(buffer);
            }

            users[*userId].is_active = true;
            users[*userId].is_loginned = true;
            return *userId;
        } else {
            sendStatusMessage(sock, 3);
            return -3;
        }
    } else {
        sendStatusMessage(sock, 3);
        return -3;
    }

}
//check
int processRegularMessage(char* mess, size_t userId, int sockId) {
    if (!users[userId].is_loginned) {
        sendStatusMessage(sockId, 2);
        return -2;
    }
    char *login = users[userId].login;
    if (getCountStringsMessage(mess) != 1) {
        sendStatusMessage(sockId, 6);
        return -6;
    }
    size_t lenBody = getLenOfStringByIndex(mess, 0);
    char* body = getStringByIndex(mess, 0);
    char buffer[BUFF_SIZE];
    createRegularMessage(buffer, login, body, lenBody);
    addMessageToHistory(buffer);
    return sendMessageToActiveUsers(buffer);
}
//check
void processLogoutMessage(size_t userId) {
    char buffer[SMALL_BUFF];
    char* login = users[userId].login;
    createNotificationLogoutMessage(buffer, userId, login);
    sendMessageToActiveUsers(buffer);
    sendStatusMessage(users[userId].socket_id, 0);
    users[userId].is_active = false;
}

void processListMessage(size_t userId) {
    int sockId = users[userId].socket_id;
    char *login = users[userId].login;
    char buffer[BUFF_SIZE];
    char names[MAX_CONNECTIONS][NAME_LEN];
    int countDiffUsers = 0;
    char tmp[NAME_LEN];
    buffer[0] = 'l';
    createEmptyMessage(buffer);
    pthread_mutex_lock(&data_mutex);
    for (size_t i = 0; i < count_of_connections_users; ++i) {
        int isNew = 1;
        for (int j = 0; j < countDiffUsers; ++j) {
            if (!strcmp(names[j], users[i].login)) {
                isNew = 0;
            }
        }
        if (isNew && users[i].is_active) {
            size_t userInd = i;
            writeIntToStr(tmp, userInd);
            addStringToMessage(buffer, tmp, 4);
            addStringToMessage(buffer, users[i].login,
                               strlen(users[i].login));
            strcpy(names[countDiffUsers++], users[i].login);
        }
    }
    pthread_mutex_unlock(&data_mutex);
    size_t allLen = getLenMessage(buffer);
    ssize_t count = send(sockId, buffer, allLen, 0);
    if (count != allLen) {
        fprintf(stderr, "Can't send list message to %s\n", login);
    }
}

void processHistoryMessage(char* mess, size_t userId) {
    char buffer[BUFF_SIZE];
    buffer[0] = 'h';
    int sockId = users[userId].socket_id;
    if (getCountStringsMessage(mess) != 1) {
        sendStatusMessage(sockId, 6);
        return;
    }
    size_t count = readIntFromStr(getStringByIndex(mess, 0));
    count = min(count, MAX_HISTORY_MESSAGE);
    int firstMess = max(messagesInHistoryCount - (int)count, 0);
    pthread_mutex_lock(&data_mutex);
    for (int i = firstMess; i < messagesInHistoryCount; ++i) {
        createEmptyMessage(buffer);
        for (size_t j = 0; j < 3; ++j) {
            char *info = getStringByIndex(history[i], j);
            size_t len = getLenOfStringByIndex(history[i], j);
            addStringToMessage(buffer, info, len);
        }
        size_t allLen = getLenMessage(buffer);
        ssize_t cntSending = send(sockId, buffer, allLen, 0);
        if (cntSending != allLen) {
            fprintf(stderr, "! Error can't send history to %s\n", users[userId].login);
        }
    }
    pthread_mutex_unlock(&data_mutex);
}

void processKickMessage(char* mess, size_t userId) {
    int sockId = users[userId].socket_id;
    if (getCountStringsMessage(mess) != 2 ||
        getLenOfStringByIndex(mess, 0) != 4) {
        sendStatusMessage(sockId, 6);
        return;
    }
    size_t kickUserLoginId = readIntFromStr(getStringByIndex(mess, 0));
    char *reason = getStringByIndex(mess, 1);
    size_t lenReason = getLenOfStringByIndex(mess, 1);
    char buffer[SMALL_BUFF];
    setTypeMessage(buffer, 'k');
    createMessageFromString(buffer, reason, lenReason);
    pthread_mutex_lock(&data_mutex);
    int countTurnCliens = 0;
    char *login = NULL;
    size_t loginInd = 0;
    for (int i = 0; i < count_of_connections_users; ++i) {
        if (i == kickUserLoginId && users[i].is_active) {
            users[i].is_active = 0;
            ++countTurnCliens;
            login = users[i].login;
            ssize_t cntSend = send(users[i].socket_id, buffer, getLenMessage(buffer), 0);
            close(users[i].socket_id);
            if (cntSend != getLenMessage(buffer)) {
                fprintf(stderr, "Can't send kick message\n");
            }
        }
    }
    pthread_mutex_unlock(&data_mutex);
    if (!countTurnCliens) {
        sendStatusMessage(sockId, 2);
    } else {
        createNotificationKickMessage(buffer, userId, login);
        sendMessageToActiveUsers(buffer);
    }
}



void processMessage(char *message, int *userId, int sockId) {
    switch (message[0]) {
        case 'r':
            processRegularMessage(message, *userId, sockId);
            break;
        case 'o':
            if (!users[*userId].is_loginned) {
                sendStatusMessage(sockId, 2);
            } else {
                processLogoutMessage(*userId);
                users[*userId].is_loginned = false;
            }
            break;
        case 'i':
            if (users[*userId].is_loginned)
                sendStatusMessage(sockId, 3);
            else
                processLogin(message, userId, sockId);
            break;
        case 'l':
            if (!users[*userId].is_loginned)
                sendStatusMessage(sockId, 2);
            else
                processListMessage(*userId);
            break;
        case 'h':
            if (!users[*userId].is_loginned)
                sendStatusMessage(sockId, 2);
            else
                processHistoryMessage(message, *userId);
            break;
        case 'k':
            if (!users[*userId].is_loginned)
                sendStatusMessage(sockId, 2);
            else if (strcmp(users[*userId].login, "root"))
                sendStatusMessage(sockId, 5);
            else
                processKickMessage(message, *userId);
        default:
            break;
    }
}

void* processClient(void * data) {
    int userId = (int)data;
    int sockid = users[userId].socket_id;
    char buffer[BUFF_SIZE];
    while (true) {
        ssize_t count = recvCommand(sockid, buffer);
        if (count <= 0) {
            if (users[userId].is_loginned) {
                users[userId].is_loginned = false;
            }
            fprintf(stderr, "recv() failed for userid: %d\n", userId);
            close(sockid);
            return NULL;
        } else {
            processMessage(buffer, &userId, sockid);
        }
    }
    close(sockid);
    return NULL;
}

int main(int argc, char **argv) {
    int options;
    while ((options = getopt(argc, argv, "p:")) != -1) {
        switch (options) {
            case 'p':
                PORT = atoi(optarg);
                break;
            default:
                break;
        }
    }
    int sockid = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockid == -1) {
        fprintf(stderr, "Socket opening failed\n");
        return 1;
    }
    struct sockaddr_in port;
    port.sin_family = AF_INET;
    port.sin_port = htons(PORT);
    port.sin_addr.s_addr = htonl(INADDR_ANY);
    int status = bind(sockid, (struct sockaddr *) &port, sizeof(port));
    if (status == -1) {
        fprintf(stderr, "Socket binding failed\n");
        return 2;
    }
    printf("Please take pass for root\n");
    size_t len_pass = 0;
    char* line_pass = NULL;
    ssize_t read = getline(&line_pass, &len_pass, stdin);
    line_pass[read - 1] = 0;
    int isRootNew = 0;
    addUser(0, "root", line_pass, &isRootNew, 0);
    users[count_of_connections_users].is_active = 0;
    ++count_of_connections_users;
    status = listen(sockid, MAX_CONNECTIONS);
    if (status == -1) {
        fprintf(stderr, "Socket listening failed\n");
        return 3;
    }
    printf("Listening to port %d...\n", ntohs(port.sin_port));
    while (true) {
        struct sockaddr client_addr;
        //int addr_len = sizeof(client_addr);
        socklen_t cli_addr_size = sizeof(client_addr);
        int sock_id_client = accept(sockid, &client_addr, &cli_addr_size);

        if (sock_id_client == -1) {
            perror("Socket accepting failed\n");
            continue;
        }

//creating threads
        init_client(count_of_connections_users, sock_id_client);
        status = pthread_create(&threads[count_of_connections_users], NULL, processClient, count_of_connections_users);
        if (status) {
            perror("Can't create input thread");
            return 10;
        }
        printf("someone with sockId:%d connected\n", count_of_connections_users);

        ++count_of_connections_users;
    }

    //}
    return 0;
}