#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>

int main(int argc, char **argv)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int portNum = 9876;
    // printf("Enter a port number :");
    // scanf("%d%*c", &portNum);

    struct sockaddr_in serveraddr, clientaddr; // family-type of address|port-deliver to correct app|s_addr-IP address
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(portNum);    // htons - creates a two byte number idn the right order
    serveraddr.sin_addr.s_addr = INADDR_ANY; // sender to any IP address (specified by client)

    bind(sockfd, (struct sockaddr *)&serveraddr,
         sizeof(serveraddr)); // ties the socket to the address
    while (1)
    { // constantly receiving
        socklen_t len = sizeof(clientaddr);
        char line[5000] = "";
        recvfrom(sockfd, line, 5000, 0,
                 (struct sockaddr *)&clientaddr, &len); // client info added to clientaddr

        printf("Received from client: %s\n", line);
        // Wait for the client to make a request
        if (strcmp(line, "") != 0)
        {
            // Array to  track if the desired window is available
            int senderWindow[5] = {0, 0, 0, 0, 0};
            // Tracks the current window in the queue
            int windowCounter = 0;
            // Track the current sequence count
            int totalCountSent = 0;
            // fileName set to the udpclient entry
            char *fileName = line;
            FILE *file;
            char *str[5]; // maybe the source of our seg fault?
            file = fopen(fileName, "r");
            printf("Got here\n");
            fgets(str[windowCounter], 255, file);

            do
            {
                printf("But not here\n");
                if (senderWindow[windowCounter] == 0)
                {
                    senderWindow[windowCounter] = 1;
                    windowCounter++;
                    sendto(sockfd, str[windowCounter], 255, 0,
                           (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                    totalCountSent++;
                    fgets(str[windowCounter], 255, file);
                    printf("%s", str[windowCounter]);
                }
                else
                {
                    sendto(sockfd, str[windowCounter], 255, 0,
                           (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                }
            } while (str != EOF);
        }
        // stringArray[counter] = strndup(str, 255);
        // counter++;
        /*
int32_t firstNum = 0;
int32_t secondNumber = 0;
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
}*/
    }
}
