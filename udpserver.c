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
	//define the socket to be used by the server
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int portNum = 9876;
    struct sockaddr_in serveraddr, clientaddr; // family-type of address|port-deliver to correct app|s_addr-IP address
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(portNum);    // htons - creates a two byte number idn the right order
    serveraddr.sin_addr.s_addr = INADDR_ANY; // sender to any IP address (specified by client)
    bind(sockfd, (struct sockaddr *)&serveraddr,
         sizeof(serveraddr)); // ties the socket to the address
	socklen_t len = sizeof(clientaddr);
	
	//define the timeout interval to be use to detect a timeout
	struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	
	//array holds the current fileSequence for each window (default 0)
    int senderWindow[5] = {0, 0, 0, 0, 0};
	
	//when an acknowledgement is received, store the acknowledgements windowCounter and file sequence
    int receivedWindowCounter = 0;
    int receivedSeqCount = 0;
    char ackLine[8];
	
//the current data to be send in the packet
    char windowValue[5][255];
    // Tracks the current window in the queue
    int windowCounter = 0;
    // Track the current sequence count of the file
    int fileSequence = 0;
    // fileName set to the udpclient entry
    char fileRequested[5000] = "";
	while(1){
		//received packet from the client with the file name
    recvfrom(sockfd, fileRequested, 5000, 0,
             (struct sockaddr *)&clientaddr, &len); // client info added to clientaddr
    char *fileName = fileRequested;
    FILE *file;
//Read the requested file
    file = fopen(fileName, "r+");
		//if the file is Null return the error message
    if (file == NULL)
    {
	    /*this should be send to the client as well, 
	    they dont know there was an error
	    */
        printf("Error! Could not open file\n");
        exit(-1);
    }

    printf("Received from client: %s\n", fileRequested);
    // Wait for the client to make a request
    if (strcmp(fileRequested, "") != 0)
    {
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
/*
Move the recurve from server here, then resend anything from window counter and up
*/
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

                memcpy(&receivedWindowCount, &ackLine[0], 4);
                memcpy(&receivedSeqCount, &ackLine[4], 4);
/*change this to reset when the server sequence = ack sequence
*/
                senderWindow[receivedWindowCounter] = 0;
                char *thing;
                thing = "";
                strcpy(&windowValue[receivedWindowCounter][0], thing);
                printf("sender window boolean : %d\nReturn window value : %d\nReturned sequence number : %d\n\n", senderWindow[receviedWindowCounter], receivedWindowCounter, receivedSeqCount);
            }
            
        } while (!feof(file));
	//while ()
	    
	//iterate through the sending window to ensure all packets have been acknowledged
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

	    /*remove this?
	    do a time out to handle the EOF
	    */
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
    }

        close(sockfd);
    }
}
