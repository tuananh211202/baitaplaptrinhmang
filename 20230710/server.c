#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>

struct myStructure
{
    int Num;
    char Name[11];
    int res; // dang ki
};
int client[10];
struct myStructure client_array[5];

char OK[] = "100 OK\n";
char In_use[] = "200 NICKNAME IN USE\n";
char Invalid[] = "201 INVALID NICK NAME\n";
char unknow[] = "202 UNKNOWN NICKNAME\n";
char denied[] = "203 DENIED\n";
char failed[] = "999 UNKNOWN ERROR\n";
int num_clientnumber = 0;

pthread_mutex_t num_clientnumber_m = PTHREAD_MUTEX_INITIALIZER;

void *client_thread(void *);
void MSG();
void PMSG();    

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

    while (1)
    {
        int clients = accept(listener, NULL, NULL);
        if (clients == -1)
        {
            perror("accept() failed");
            continue;
            ;
        }
        printf("Ket noi moi: %d\n", clients);
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &clients);
        pthread_detach(thread_id);
    }
    close(listener);
    return 0;
}

void *client_thread(void *param)
{
    int clients = *(int *)param;
    char bufrecv[50];
    while (1)
    {
        char ID1[4];
        char Name1[10];
        char test[5];
        int ret2 = recv(clients, bufrecv, sizeof(bufrecv), 0);
        bufrecv[ret2] = 0;
        int check = sscanf(bufrecv, "%s %s %s", ID1, Name1, test);
        client_array[num_clientnumber].res = 0;
        if (check == 2 && strncmp(ID1, "JOIN", 4) == 0)
        {   
            for(int i = 0; i < num_clientnumber; i++) {
                if(strcmp(Name1, client_array[i].Name) == 0) {
                    printf("Name1: %s\n", Name1);
                    client_array[num_clientnumber].res = 2;
                    send(clients, In_use, sizeof(In_use), 0);
                }
            }
            if (client_array[num_clientnumber].res != 2)
            {
                pthread_mutex_lock(&num_clientnumber_m);
                client_array[num_clientnumber].res = 1;
                printf("%s\n", bufrecv);
                client_array[num_clientnumber].Num = clients;
                strcpy(client_array[num_clientnumber].Name, Name1);
                client[num_clientnumber] = clients;
                num_clientnumber++;
                pthread_mutex_unlock(&num_clientnumber_m);
                send(clients, OK, sizeof(OK), 0);
                printf("%d\n", num_clientnumber);
                break;
            }
            
        }
        else
        {
            send(clients, Invalid, sizeof(Invalid), 0);
        }
    }
    int a = num_clientnumber - 1;
    while (1)
    {   
        int ret2 = recv(clients, bufrecv, sizeof(bufrecv), 0);
        bufrecv[ret2] = 0;
        char ID1[5];
        char test[4];
        sscanf(bufrecv, "%s %s %s", ID1, test);
        if(strncmp(ID1, "MSG", 3) == 0) {
            MSG(clients, bufrecv);
        }

        else if(strncmp(ID1, "PMSG", 4) == 0) {
            PMSG(clients);
        }
        else if(strncmp(ID1, "QUIT", 4) == 0) {
            send(clients, OK, sizeof(OK), 0);
            break;
        }
        else {
            send(clients, failed, sizeof(failed), 0);
        }

    }
    
}
const char s[2] = " ";
void MSG(int clients, char buf[]) {
    char STA[] = strtok(buf, s);
    for(int i = 0; i < num_clientnumber; i++) {
        if (client[i] != clients)
        send(client[i], STA, sizeof(STA), 0);
    }
    send(clients, OK, sizeof(OK), 0);
    
}

void PMSG(int client, char buf) {
    char ID1[5];
    char Name[11];
    char test[4];
    sscanf(buf, "%s %s %s", ID1, Name, test);
}
