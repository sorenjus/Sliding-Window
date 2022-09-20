#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>

int main(int argc, char **argv) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int portNum = 9876;
    //printf("Enter a port number :");
    //scanf("%d%*c", &portNum);

    struct sockaddr_in serveraddr, clientaddr;//family-type of address|port-deliver to correct app|s_addr-IP address
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(portNum);//htons - creates a two byte number idn the right order
    serveraddr.sin_addr.s_addr = INADDR_ANY;//sender to any IP address (specified by client)

    bind(sockfd, (struct sockaddr *) &serveraddr,
         sizeof(serveraddr));//ties the socket to the address
    while (1) { //constantly receiving
        socklen_t len = sizeof(clientaddr);
        char line[5000];
        int32_t firstNum = 0;
        int32_t secondNumber = 0;
        recvfrom(sockfd, line, 5000, 0,
                 (struct sockaddr *) &clientaddr, &len);//client info added to clientaddr
        memcpy(&firstNum, &line[0], 4);
        memcpy(&secondNumber, &line[4], 4);
        printf("Got from client: %d\n"PRId32, firstNum);
        printf("Got from client: %d\n"PRId32, secondNumber);
        int32_t result = firstNum + secondNumber;
        int32_t overflow = 0;
        if ((firstNum > 0 && secondNumber > 0 && result < 0) || (firstNum < 0 && secondNumber < 0 && result > 0)) {
            overflow = 1;
            memcpy(&line[4], &overflow, sizeof(int));
            sendto(sockfd, line, (sizeof(int) * 2), 0,
                   (struct sockaddr *) &clientaddr, sizeof(clientaddr));
        }
        else {
            memcpy(&line, &result, sizeof(int));
            sendto(sockfd, line, sizeof(int), 0,
                   (struct sockaddr *) &clientaddr, sizeof(clientaddr));
        }
    }
}
