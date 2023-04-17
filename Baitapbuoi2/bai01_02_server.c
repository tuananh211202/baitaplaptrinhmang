#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int countString(char *str){
    int i, ans = 0;
    int len = strlen(str);
    char tmp[10];
    if(len > 256) len = 256;
    for(i = 0;i + 10 < len;i++){
        strncpy(tmp, str + i, 10);
        if (strcmp(tmp, "0123456789") == 0) ans++;
    }
    return ans;
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
    char tmpStr[16];
    strcpy(tmpStr, "");

    int ans = 0;

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

        char suffix[10];
        // printf("%d bytes received\n", ret);
        ans += countString(buf);
    
        strncpy(suffix, buf, 8);
        strcat(tmpStr, suffix);

        ans += countString(tmpStr);

        int len = strlen(buf);
        if(len > 256) len = 256;
        strncpy(suffix, buf + len - 8, 8);
        strcpy(tmpStr, suffix);
    }

    printf("This str appears %d times.", ans);
    
    close(client);
    close(listener);
}
