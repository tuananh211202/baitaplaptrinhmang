#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void doIt(char *str){
    char *token = strtok(str, " ");
    int cnt = 0;
    while(token != NULL){
        if(cnt == 0){
            printf("Ten may tinh:%s\n", token);
        }
        else if(cnt == 1){
            printf("So o dia:%s\n", token);
        }
        else if(cnt % 2 == 0){
            printf("Ten o dia:%s - ", token);
        }else printf("%sGB\n", token);
        token = strtok(NULL, " ");
        cnt++;
    }
}

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(addr);

    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    printf("Client IP: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    char *msg = "Hello client\n";
    send(client, msg, strlen(msg), 0);

    char buf[256];

    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            printf("Ket noi bi dong.\n");
            break;    
        }
        if (ret < sizeof(buf))
            buf[ret] = 0;

        doIt(buf);
        buf[0] = '\0';
    }
    
    close(client);
    close(listener);
}
