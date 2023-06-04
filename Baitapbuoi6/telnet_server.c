#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

void signalHandler(int signo)
{
    int pid = wait(NULL);
    printf("Child %d terminated.\n", pid);
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

    signal(SIGCHLD, signalHandler);

    FILE *f = fopen("database.txt", "rb");

    while (1)
    {
        printf("Waiting for new client...\n");
        int client = accept(listener, NULL, NULL);
        if (fork() == 0)
        {
            // Tien trinh con
            close(listener);

            // Xu ly ket noi tu client

            char buf[256];
            int isLogin = 0;
            while (1)
            {
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                    break;
                buf[ret] = 0;
                printf("Received from %d: %s\n", client, buf);

                // xu ly dang nhap
                if(isLogin == 0){
                    char line[100];
                    int k = 0;
                    while (fgets(line, 100, f) != NULL)
                    {
                        if (strncmp(line, buf, strlen(buf) - 1) == 0)
                        {
                            k = 1;
                            break;
                        }
                    }
                    if(k){
                        isLogin = 1;
                        char *msg = "Dang nhap hoan thanh. Nhap lenh can xu ly:\n";
                        send(client, msg, strlen(msg), 0);
                    } else {
                        char *msg = "Nhap sai. Nhap lai.\n";
                        send(client, msg, strlen(msg), 0);
                    }
                } else {
                    ret = system(buf);
                    if (ret == 0)
                    {
                        char *msg = "Hoan Thanh!\n";
                        send(client, msg, strlen(msg), 0);
                    }
                    else
                    {
                        char *msg = "Lỗi khi thực thi lệnh";
                        send(client, msg, strlen(msg), 0);
                    }
                }
                
            }

            close(client);
            exit(0);
        }

        // Tien trinh cha
        close(client);
    }
    
    close(listener); 
    fclose(f);   

    return 0;
}