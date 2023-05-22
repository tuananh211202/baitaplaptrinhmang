#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>

char toUpper(char charx){
    if(charx >= 'a' && charx <='z') charx = charx-32;
    return charx;
}

char toLower(char charx){
    if(charx >= 'A' && charx <= 'Z') charx = charx + 32;
    return charx;
}
 
//Hàm chuẩn hóa xâu
void chuanHoa(char a[])
{
    int n = strlen(a); //Lấy độ dài xâu
 
//Xóa khoảng trắng đầu xâu
    for(int i=0;i<n;i++){
        if(a[i]==' '){
            for(int j=i;j<n-1;j++){
                a[j] = a[j+1];
            }
            a[n-1] = '\0';
            i--;
            n--;
        }
        else break;
    }
     
 
//Xóa khoảng trắng cuối xâu
    for(int i=n-1;i>=0;i--){
        if(a[i]==' '){
            a[i]='\0';
            n--;
        }
        else break;
    }
     
    //Xóa khoảng trắng không hợp lệ(khoảng trắng bị thừa)
    for(int i=1;i<n-1;i++)
    {
        if(a[i]==a[i+1]){
            for(int j=i;j<n-1;j++){
                a[j] = a[j+1];
            }
            a[n-1]='\0';
            i--;
            n--;
        }
    }
     
//In hoa ký tự đầu tiên 
    for(int i = 0;i < n;i++) a[i] = toLower(a[i]);
    a[0] =  toUpper(a[0]); 
    for(int i = 0;i + 1 < strlen(a);i++) if(a[i] == ' ') a[i+1] = toUpper(a[i+1]);
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

    struct pollfd fds[64];
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
                nfds++;

                printf("New client connected: %d\n", client);
                char msg[80];
                sprintf(msg, "Xin chào. Hiện có %d clients đang kết nối.", nfds - 1);
                send(client, msg, strlen(msg), 0);
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
                    if (i < nfds - 1)
                        fds[i] = fds[nfds - 1];
                    nfds--;
                    i--;
                }
                else
                {
                    buf[ret] = 0;
                    printf("Received from %d: %s\n", fds[i].fd, buf);
                    chuanHoa(buf);
                    send(fds[i].fd, buf, strlen(buf), 0);
                }
            }
    }
    
    close(listener);    

    return 0;
}