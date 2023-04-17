#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void removeNewLine(char *str){
    strtok(str, "\n");
}

int main()
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000);

    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("connect() failed");
        return 1;
    }

    char buf[256];
    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        printf("Connection closed\n");
        return 1;
    }

    if (ret < sizeof(buf))
        buf[ret] = 0;
    printf("%d bytes received: %s\n", ret, buf);

    while (1)
    {
        strcpy(buf, "");
        char computerName[100];
        printf("Ten may tinh:");
        fgets(computerName, sizeof(computerName), stdin);
        
        if (strncmp(computerName, "exit", 4) == 0)
            break;

        removeNewLine(computerName);
        strcpy(buf, computerName);
        strcat(buf, " ");

        printf("So o dia:");
        char cnt[3];
        int num;
        fgets(cnt, sizeof(cnt), stdin);
        removeNewLine(cnt);
        num = atoi(cnt);
        strcat(buf, cnt);

        for(int i = 0;i < num;i++){
            char disk[10];
            printf("Ten o dia - Kich thuoc o dia(GB):");
            fgets(disk, sizeof(disk), stdin);
            removeNewLine(disk);
            strcat(buf, " ");
            strcat(buf, disk);
    
        }

        send(client, buf, strlen(buf), 0);
        buf[0] = '\0';
    }

    close(client);
}
