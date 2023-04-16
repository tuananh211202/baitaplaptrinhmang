#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char *argv[])
{
    char *ip_address = argv[1];
    char *port = argv[2];
    int portNumber = atoi(port); 

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_address);
    addr.sin_port = htons(portNumber);

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
        char msg[256];
        strcpy(msg, ip_address);
        strcat(msg, " ");

        printf("MSSV - Ho va ten - Ngay sinh - Diem trung binh:");
        fgets(buf, sizeof(buf), stdin);
       
        if (strncmp(buf, "exit", 4) == 0)
            break;

        time_t timer;
        char currentTime[26];
        struct tm* tm_info;

        timer = time(NULL);
        tm_info = localtime(&timer);

        strftime(currentTime, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        
        strcat(msg, currentTime);
        strcat(msg, " ");
        strcat(msg, buf);

        send(client, msg, strlen(msg), 0);

        printf("\n");        
    }

    close(client);
}
