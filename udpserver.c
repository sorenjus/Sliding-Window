#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char **argv)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int portNum = 9876;
    // printf("Enter a port number :");
    // scanf("%d%*c", &portNum);
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    
    struct sockaddr_in serveraddr, clientaddr; // family-type of address|port-deliver to correct app|s_addr-IP address
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(portNum);    // htons - creates a two byte number idn the right order
    serveraddr.sin_addr.s_addr = INADDR_ANY; // sender to any IP address (specified by client)

    bind(sockfd, (struct sockaddr *)&serveraddr,
         sizeof(serveraddr)); // ties the socket to the address
    int senderWindow[5] = {0, 0, 0, 0, 0};
    // int senderReceipt[6] = {0, 0, 0, 0, 0, 0};
    // int clientAcknowledgements[6];
    int currentWindowCounter = 0;
    int currentCount = 0;
    char ackLine[8];
    char windowValue[5][255];
    // Tracks the current window in the queue
    int windowCounter = 0;
    // Track the current sequence count
    int fileSequence = 0;
    // fileName set to the udpclient entry
    socklen_t len = sizeof(clientaddr);
    char fileRequested[5000] = "";
    recvfrom(sockfd, fileRequested, 5000, 0,
             (struct sockaddr *)&clientaddr, &len); // client info added to clientaddr
    char *fileName = fileRequested;
    FILE *file;
    file = fopen(fileName, "r+");
    if (file == NULL)
    {
        printf("Error! Could not open file\n");
        exit(-1);
    }

    printf("Received from client: %s\n", fileRequested);
    // Wait for the client to make a request
    if (strcmp(fileRequested, "") != 0)
    {
        // Array to  track if the desired window is available
        // printf("%s\n", &windowValue[windowCounter]);

        do
        {
            if (senderWindow[windowCounter] == 0)
            {
                fgets(&windowValue[windowCounter][0], 255, file);
                char line[263] = "";
                memcpy(&line[0], &windowCounter, 4);
                memcpy(&line[4], &fileSequence, 4);
                strcpy(&line[8], &windowValue[windowCounter][0]);
                // if (totalCountSent < 5)
                // {
                // printf("%s\n", &windowValue[windowCounter]);
                sendto(sockfd, line, 263, 0,
                       (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                // printf("%*s\n", &line);
                printf("Sent packet at window %d\n", windowCounter);
                printf("Sequence number : %d\n", fileSequence);
                printf("packet contents : %s\n", windowValue[windowCounter]);
                senderWindow[windowCounter] = fileSequence;
                // senderReceipt[windowCounter] = 1;
                // senderReceipt[5] = windowCounter;
                // sendto(sockfd, senderReceipt, 24, 0,
                //      (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                windowCounter++;
                if (windowCounter == 5)
                {
                    windowCounter = 0;
                    // memset(senderReceipt, 0, 6 * sizeof(int));
                    // memset(senderWindow, 0, 5 * sizeof(int));
                }
                fileSequence++;
            }
            else
            {
                char line[263] = "";
                memcpy(&line[0], &windowCounter, 4);
                memcpy(&line[4], &senderWindow[windowCounter], 4);
                strcpy(&line[8], &windowValue[windowCounter][0]);
                sendto(sockfd, line, 263, 0,
                       (struct sockaddr *)&clientaddr, sizeof(clientaddr));
            }
            printf("receiving\n");
            int n = recvfrom(sockfd, ackLine, 9, 0,
                         (struct sockaddr *)&clientaddr, &len);
            if (n == -1) {
            	if (errno == EWOULDBLOCK) {
            		printf("Timed out while waiting to receive\n");
            	}
            } else {
                memcpy(&currentWindowCounter, &ackLine[0], 4);
                memcpy(&currentCount, &ackLine[4], 4);
                senderWindow[currentWindowCounter] = 0;
                char *thing;
                thing = "";
                strcpy(&windowValue[currentWindowCounter][0], thing);
                printf("sender window boolean : %d\nReturn window value : %d\nReturned sequence number : %d\n\n", senderWindow[currentWindowCounter], currentWindowCounter, currentCount);
            }
            
        } while (!feof(file));
	//while ()
	// checking our file sequence to make them all zero
    do{
    for(int i = 0; i < 5; i++){
        if(senderWindow[i] != 0){
            char line[263] = "";
                memcpy(&line[0], &i, 4);
                memcpy(&line[4], &senderWindow[i], 4);
                strcpy(&line[8], &windowValue[i][0]);
                // if (totalCountSent < 5)
                // {
                // printf("%s\n", &windowValue[windowCounter]);
                sendto(sockfd, line, 263, 0,
                       (struct sockaddr *)&clientaddr, sizeof(clientaddr));
             int n = recvfrom(sockfd, ackLine, 9, 0,
                         (struct sockaddr *)&clientaddr, &len);
            if (n == -1) {
            	if (errno == EWOULDBLOCK) {
            		printf("Timed out while waiting to receive\n");
            	}
            } else {
                memcpy(&currentWindowCounter, &ackLine[0], 4);
                memcpy(&currentCount, &ackLine[4], 4);
                senderWindow[currentWindowCounter] = 0;
                char *thing;
                thing = "";
                strcpy(&windowValue[currentWindowCounter][0], thing);
                printf("sender window boolean : %d\nReturn window value : %d\nReturned sequence number : %d\n\n", senderWindow[currentWindowCounter], currentWindowCounter, currentCount);
            }
        }

    }
    } while(senderWindow[0] != 0 && senderWindow[1] != 0 && senderWindow[2] != 0 &&
        senderWindow[3] != 0 && senderWindow[4] != 0);

    do{
        printf("Got here\n");
        char *str = "EOF";
        sendto(sockfd, str, 263, 0,
               (struct sockaddr *)&clientaddr, sizeof(clientaddr));
        char line[264] = "";
    // receive the packet from the server
    recvfrom(sockfd, line, 255, 0,
             (struct sockaddr *)&clientaddr, &len);
    if (strstr(line, "EOF"))
    {
      break;
    }
    } while(1);
	// another while loop after we exit
	// keep running until we receive back from the client "EOF"
        fclose(file);

        close(sockfd);
    }
}
