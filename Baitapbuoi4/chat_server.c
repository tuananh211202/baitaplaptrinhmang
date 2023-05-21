#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>

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

    struct pollfd fds[64];
    int isLogin[100];
    char *userIds[100];
    int nfds = 1;

    fds[0].fd = listener;
    fds[0].events = POLLIN;

    char buf[256];

    while (1)
    {
        int ret = poll(fds, nfds, -1);
        if (ret < 0)
        {
            perror("poll() failed");
            break;
        }

        if (fds[0].revents & POLLIN)
        {
            int client = accept(listener, NULL, NULL);
            if (nfds == 64)
            {
                // Tu choi ket noi
                close(client);
            }
            else
            {
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;
                isLogin[client] = 0;
                nfds++;

                printf("New client connected: %d\n", client);
            }
        }

        for (int i = 1; i < nfds; i++)
            if (fds[i].revents & POLLIN)
            {
                ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    close(fds[i].fd);
                    // Xoa khoi mang
                    isLogin[fds[i].fd] = 0;
                    if (i < nfds - 1)
                        fds[i] = fds[nfds - 1];
                    nfds--;
                    i--;
                }
                else
                {
                    buf[ret] = 0;
                    printf("Received from %d: %s\n", fds[i].fd, buf);
                    if(isLogin[fds[i].fd] == 0){
                        char cmd[32], id[32], tmp[32];
                        ret = sscanf(buf, "%s%s%s", cmd, id, tmp);
                        if (ret == 2)
                        {
                            if (strcmp(cmd, "client_id:") == 0)
                            {
                                char *msg = "Dung cu phap. Hay nhap tin nhan de chuyen tiep.\n";
                                send(fds[i].fd, msg, strlen(msg), 0);
                                isLogin[fds[i].fd] = 1;
                                userIds[fds[i].fd] = malloc(strlen(id) + 1);
                                strcpy(userIds[fds[i].fd], id);
                                
                            }
                        } else {
                            char *msg = "Sai tham so. Hay nhap lai.\n";
                            send(fds[i].fd, msg, strlen(msg), 0);
                        }
                    } else {
                        char sendbuf[256];

                        strcpy(sendbuf, userIds[fds[i].fd]);
                        strcat(sendbuf, ": ");
                        strcat(sendbuf, buf);
                        for (int j = 1;j < nfds;j++) if(j != i && isLogin[fds[j].fd]) send(fds[j].fd, sendbuf, strlen(sendbuf), 0);
                    }
                }
            }
    }
    
    close(listener);    

    return 0;
}