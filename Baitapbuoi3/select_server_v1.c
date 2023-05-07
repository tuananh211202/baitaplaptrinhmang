#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>

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

    fd_set fdread;
    
    int clients[64];
    char name[64][32];
    const char question[] = "Server: Input id: name";
    const char fail[] = "Server: Wrong struct";
    const char success[] = "Server: Connect to chat server";
    int num_clients = 0;
    
    char buf[200];
    char sbuf[256];
    char client_id[50];
    char client_name[50];
    
    while (1)
    {
        // Xóa tất cả socket trong tập fdread
        FD_ZERO(&fdread);

        // Thêm socket listener vào tập fdread
        FD_SET(listener, &fdread);
        int maxdp = listener + 1;

        // Thêm các socket client vào tập fdread
        for (int i = 0; i < num_clients; i++)
        {
            FD_SET(clients[i], &fdread);
            if (maxdp < clients[i] + 1)
                maxdp = clients[i] + 1;
        }

        // Chờ đến khi sự kiện xảy ra
        int ret = select(maxdp, &fdread, NULL, NULL, NULL);

        if (ret < 0)
        {
            perror("select() failed");
            return 1;
        }

        // Kiểm tra sự kiện có yêu cầu kết nối
        if (FD_ISSET(listener, &fdread))
        {
            int client = accept(listener, NULL, NULL);
            clients[num_clients] = client;
            strcpy(name[num_clients], "");
            num_clients++;
        }

        // Kiểm tra sự kiện có dữ liệu truyền đến socket client
        for (int i = 0; i < num_clients; i++)
            if (FD_ISSET(clients[i], &fdread))
            {
                ret = recv(clients[i], buf, sizeof(buf), 0);
                buf[ret] = 0;
                if (ret <= 0)
                {
                    // TODO: Client đã ngắt kết nối, xóa client ra khỏi mảng
                    printf("%s out\n", name[i]);
                    for(int j = i;j < num_clients;j++){
                        clients[j] = clients[j+1];
                        strcpy(name[j], name[j+1]);
                    }
                    num_clients--;
                    for(int j = 0;j < num_clients;j++)
                        printf("%s ", name[j]);
                    printf("\n");
                    continue;
                }
                if(strcmp(name[i], "") == 0){
                    // Check type (client_id: client_name)
                    if(sscanf(buf, "%[^:]:%s", client_id, client_name) == 2){
                        strcpy(name[i], client_id);
                    } else {
                        send(clients[i], question, sizeof(question), 0);
                        continue;
                    }
                } else{
                    printf("Du lieu nhan tu %s: %s\n", name[i], buf);
                    strcpy(sbuf, name[i]);
                    strcat(sbuf, ": ");
                    strcat(sbuf, buf);
                    for(int j = 0;j < num_clients;j++) if(j != i && strcmp(name[j], "") != 0){
                        send(clients[j], sbuf,sizeof(sbuf), 0);
                    }
                }
            }
    }

    close(listener);    

    return 0;
}
