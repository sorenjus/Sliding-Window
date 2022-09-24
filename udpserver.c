#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>

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
    int senderWindow[5] = {0, 0, 0, 0, 0};
    int senderReceipt[6] = {0, 0, 0, 0, 0, 0};
    int clientAcknowledgements[6];
    char windowValue[255];
    // Tracks the current window in the queue
    int windowCounter = 0;
    // Track the current sequence count
    int totalCountSent = 0;
    // fileName set to the udpclient entry
    socklen_t len = sizeof(clientaddr);
    char line[5000] = "";
    recvfrom(sockfd, line, 5000, 0,
             (struct sockaddr *)&clientaddr, &len); // client info added to clientaddr
    char *fileName = line;
    FILE *file;
    file = fopen(fileName, "r+");
    if (file == NULL)
    {
        printf("Error! Could not open file\n");
        exit(-1);
    }
    // while (1)
    // { // constantly receiving

    printf("Received from client: %s\n", line);
    // Wait for the client to make a request
    if (strcmp(line, "") != 0)
    {
        // Array to  track if the desired window is available
        fgets(&windowValue[windowCounter], 255, file);
        printf("%s\n", &windowValue[windowCounter]);
        do
        {
            if (windowCounter == 5 && totalCountSent == 5)
            {
                windowCounter = 0;
                totalCountSent = 0;
                memset(senderReceipt, 0, 6 * sizeof(int));
                memset(senderWindow, 0, 5 * sizeof(int));
            }
            if (senderWindow[windowCounter] == 0)
            {
                // if (totalCountSent < 5)
                // {
                // printf("%s\n", &windowValue[windowCounter]);
                sendto(sockfd, &windowValue[windowCounter], 255, 0,
                       (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                printf("%s\n", &windowValue[windowCounter]);
                senderWindow[windowCounter] = 1;
                senderReceipt[windowCounter] = 1;
                senderReceipt[5] = windowCounter;
                sendto(sockfd, senderReceipt, 24, 0,
                       (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                windowCounter++;
                totalCountSent++;
                // fgets(&windowValue[windowCounter], 255, file);
            }
            else
            {
                printf("%s\n", &windowValue[windowCounter]);
                sendto(sockfd, &windowValue[windowCounter], 255, 0,
                       (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                printf("Sent packet at window %d\n", windowCounter);
                senderWindow[windowCounter] = 1;
                senderReceipt[windowCounter] = 1;
                senderReceipt[5] = windowCounter;
                sendto(sockfd, senderReceipt, 24, 0,
                       (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                totalCountSent++;
                windowCounter++;
            }
            recvfrom(sockfd, clientAcknowledgements, 24, 0,
                     (struct sockaddr *)&clientaddr, &len);

        } while (fgets(&windowValue[windowCounter], 255, file));
        printf("Got here\n");
        char str[] = "EOF";
        sendto(sockfd, &str, sizeof(str), 0,
               (struct sockaddr *)&clientaddr, sizeof(clientaddr));
        fclose(file);
        // }
        // close(sockfd);
    }
}
