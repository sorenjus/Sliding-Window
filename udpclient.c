/*
char addrIP;
short portNum = 0;

printf("Enter an IP Address :");
  //fgets(&addrIP,5000,stdin);
  scanf("%s", &addrIP);
  printf("Enter a port number :");
  scanf("%hd%*c", &portNum);
printf("%s%hd", &addrIP,portNum);
*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>

int main(int argc, char** argv){
  int sockfd = socket(AF_INET,SOCK_DGRAM,0);
  if(sockfd < 0){
    printf("There was an error creating the socket\n");
    return 1;
  }

  char addrIP[20] = "";
  int portNum = 9876;

  printf("Enter an IP Address :");
  fgets(&addrIP,5000,stdin);
  //printf("Enter a port number :");
  //scanf("%d%*c", &portNum);
  struct sockaddr_in serveraddr;
  serveraddr.sin_family=AF_INET;
  serveraddr.sin_port=htons(portNum);
  serveraddr.sin_addr.s_addr=inet_addr(&addrIP);

  printf("Enter a number: ");
  char line[5000];
  int32_t firstNum = 5; //guarantees a 32 bit integer
  int32_t secondNum = 1;
  int32_t overflowCheck = 0;
  int result = 0;
  scanf("%"SCNd32, &firstNum);
    memcpy(&line[0], &firstNum, 4);
    printf("Enter a number: ");
   scanf("%"SCNd32, &secondNum);
    memcpy(&line[4], &secondNum, 4);
  sendto(sockfd,line,sizeof(int) * 2,0,
	 (struct sockaddr*)&serveraddr,sizeof(serveraddr));

    while(1) { //constantly receiving
        socklen_t len = sizeof(serveraddr);
        char line[5000] = "\0";
        bind(sockfd,(struct sockaddr*)&serveraddr,
             sizeof(serveraddr));
        recvfrom(sockfd, line, 5000, 0,
                 (struct sockaddr *) &serveraddr, &len);//client info added to clientaddr
        memcpy(&result, &line[0], 4);

        memcpy(&overflowCheck, &line[4], 4);
        if( overflowCheck == 1 ){
            printf("Got from server: Overflow Error\n");
        }
        else {
            printf("Got from server: %d\n"PRId32, result);
        }
        break;
    }

    close(sockfd);
    
  return 0;
}
