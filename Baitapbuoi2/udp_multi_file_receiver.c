#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

const int mod = 1e6 + 3;
const int base = 331;

struct senderStruct{
    char ip_addr[50];
    int port;
};
    
int encode(struct senderStruct tmp){
    char ip_addr[50];strcpy(ip_addr, tmp.ip_addr);
    int port = tmp.port;
    int code = 0, i, p1 = port / base + 1, p2 = port % base + 1;
    for(i = 0;i < strlen(ip_addr);i++) 
        code = (code * base + (int)ip_addr[i]) % mod;
    code = (code * base + p1) % mod;
    code = (code * base + p2) % mod;
    return code;
}

int main(int argc, char *argv[])
{
    int portNumber = atoi(argv[1]);

    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(portNumber);

    bind(receiver, (struct sockaddr *)&addr, sizeof(addr));

    char buf[256];
    struct sockaddr_in sender_addr;
    int sender_addr_len = sizeof(sender_addr);
    
    FILE *fptr;
    char fileName[50];

    // Receive file content
    while (1)
    {
        int ret = recvfrom(receiver, buf, sizeof(buf), 0,
            (struct sockaddr *)&sender_addr, &sender_addr_len);
        if (ret < sizeof(buf))
            buf[ret] = 0; 

        struct senderStruct tmp;
        strcpy(tmp.ip_addr, inet_ntoa(sender_addr.sin_addr));
        tmp.port = ntohs(sender_addr.sin_port);
        sprintf(fileName, "%d.txt", encode(tmp));

        fptr = fopen(fileName, "a");
        fprintf(fptr, "%s", buf);
        printf("Save %d bytes from %s:%d to %s : %s\n", ret, 
            inet_ntoa(sender_addr.sin_addr), ntohs(sender_addr.sin_port), fileName, buf);
        fclose(fptr);
    }
}
