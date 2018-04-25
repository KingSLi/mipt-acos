#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include <unistd.h>


#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int echoServPort = 1337;
char LOCALIP[] = "127.0.0.1";
const int BUF_SIZE = 2048;

void processConnection(int sockid) {
    char buffer_to_rec[BUF_SIZE];
    char *buffer_to_std = NULL;
    size_t len = 0;
    while(1)  {
        int count = recv(sockid, buffer_to_rec, BUF_SIZE, 0);
        if (count == -1) {
            fprintf(stderr, "recv() failed\n");
            close(sockid);
            return;
        }
        printf("%s", buffer_to_rec);

        getline(&buffer_to_std, &len, stdin);
        count = send(sockid, buffer_to_std, len, 0);
        if (count == -1) {
            fprintf(stderr, "send() failed\n");
            close(sockid);
            return;
        }


        free(buffer_to_std);
        len = 0;
    }
}

int main(int argc, char **argv) {
    if(argc != 3){
        fprintf(stderr, "Wrong arguments!\n");
        exit(0);
    }
    int sock_id = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_id == -1) {
        fprintf(stderr, "Socket opening failed\n");
        return 1;
    }
    strcpy(LOCALIP, argv[1]);
    echoServPort = atoi(argv[2]);
    struct sockaddr_in server_add;
    server_add.sin_family = AF_INET; /* Internet address family */
    server_add.sin_addr.s_addr = inet_addr(LOCALIP);//AF_LOCAL; /* Server IP address*/
    server_add.sin_port = htons(echoServPort);
    int status = connect(sock_id, (struct sockaddr *)&server_add, sizeof(server_add));
    if (status == -1) {
        fprintf(stderr, "Socket connecting failed\n");
        return 2;
    } else {
        printf("server connected\n");

    }
    processConnection(sock_id);

}