#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  char *ip_address = argv[1];
  int portNumber = atoi(argv[2]);
  char *fileName = argv[3];
  char endFile[4];
  strcpy(endFile, "###");

  FILE *fptr;

  if ((fptr = fopen(fileName, "r")) == NULL) {
    printf("Wrong file name!!!\n");
    return 0;
  }

  int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip_address);
  addr.sin_port = htons(portNumber);

  char buf[256];
  // while (1) {
  //   printf("Enter string: ");
  //   fgets(buf, sizeof(buf), stdin);
  //   int ret = sendto(sender, buf, strlen(buf), 0, (struct sockaddr *)&addr,
  //                    sizeof(addr));
  //   printf("%d bytes sent.\n", ret);
  // }
  
  int ret = sendto(sender, fileName, strlen(fileName), 0, (struct sockaddr *)&addr, sizeof(addr));

  while (fgets(buf, sizeof(buf), fptr)) {
    ret = sendto(sender, buf, strlen(buf), 0, (struct sockaddr *)&addr,
                     sizeof(addr));
  }
  
  ret = sendto(sender, endFile, strlen(endFile), 0, (struct sockaddr *)&addr, sizeof(addr));
  
  fclose(fptr);
}
