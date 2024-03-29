#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

bool running = true;

// Authors : Justin Sorensen, Meghan Harris

int main(int argc, char **argv)
{
    // define the socket to be used by the server
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int portNum = 9876;
    printf("Enter a port number :");
    scanf("%d%*c", &portNum);
    struct sockaddr_in serveraddr, clientaddr; // family-type of address|port-deliver to correct app|s_addr-IP address
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(portNum);    // htons - creates a two byte number idn the right order
    serveraddr.sin_addr.s_addr = INADDR_ANY; // sender to any IP address (specified by client)
    bind(sockfd, (struct sockaddr *)&serveraddr,
         sizeof(serveraddr)); // ties the socket to the address
    socklen_t len = sizeof(clientaddr);

    // define the timeout interval to be use to detect a timeout
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    // array holds the current fileSequence for each window (default 0)
    int senderWindow[5] = {-1, -1, -1, -1, -1};

    // when an acknowledgement is received, store the acknowledgements windowCounter and file sequence
    int receivedWindowCounter = 0;
    int receivedSeqCount = 0;
    char ackLine[9];

    // the current data to be sent in the packet
    char windowValue[5][255];
    // Tracks the current window in the queue
    int windowCounter = 0;
    // Track the current sequence count of the file
    int fileSequence = 0;
    while (running)
    {
        // fileName set to the udpclient entry
        char fileRequested[5000] = "";

        // received packet from the client with the file name, or timeout while waiting
        int f = recvfrom(sockfd, fileRequested, 5000, 0,
                         (struct sockaddr *)&clientaddr, &len); // client info added to clientaddr
        if (f == -1)
        {
            if (errno == EWOULDBLOCK)
            {
                printf("Waiting for file name\n\n");
            }
        }
        else
        {
            if (strcmp(fileRequested, "") != 0)
            {
                char *fileName = fileRequested;
                FILE *file;
                // detect if the file is a txt or binary file
                bool text = strstr(fileName, ".txt");

                if (text) // read a text file
                {
                    file = fopen(fileName, "r+");
                }
                else // read a binary file
                {
                    file = fopen(fileName, "rb");
                }

                // if the file is Null return the error message, exit, and notify the client
                if (file == NULL)
                {
                    printf("Error! Could not open file\n");
                    char line[263] = "Error! Could not open file.";
                    sendto(sockfd, line, 263, 0,
                           (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                    exit(-1);
                }

                printf("Received from client: %s\n", fileRequested);

                do
                {
                    if (senderWindow[windowCounter] == -1 && !feof(file))
                    {
                        if (text && !feof(file))
                        {
                            fgets(&windowValue[windowCounter][0], 255, file);
                        }
                        else if (!text && !feof(file))
                        {
                            fread(&windowValue[windowCounter][0], sizeof(windowValue[windowCounter][0]), 1, file); // read
                        }

                        senderWindow[windowCounter] = fileSequence;

                        char line[263] = "";
                        memcpy(&line[0], &windowCounter, 4);
                        memcpy(&line[4], &senderWindow[windowCounter], 4);
                        memcpy(&line[8], &windowValue[windowCounter][0], 255);
                        sendto(sockfd, line, 263, 0,
                               (struct sockaddr *)&clientaddr, sizeof(clientaddr));

                        printf("Sent packet at window %d\n", windowCounter);
                        printf("Sequence number : %d\n", senderWindow[windowCounter]);
                        printf("packet contents : %s\n", windowValue[windowCounter]);

                        windowCounter++;
                        fileSequence++;

                        if (windowCounter == 5)
                        {
                            windowCounter = 0;
                        }
                    }
                    else
                    {
                        int n = recvfrom(sockfd, ackLine, 9, 0,
                                         (struct sockaddr *)&clientaddr, &len);
                        if (n == -1)
                        {
                            if (errno == EWOULDBLOCK)
                            {
                                printf("Timed out while waiting to receive\nResending data lines");

                                for (int i = 0; i < 5; ++i)
                                {
                                    char line[263] = "";
                                    memcpy(&line[0], &i, 4);
                                    memcpy(&line[4], &senderWindow[i], 4);
                                    memcpy(&line[8], &windowValue[i][0], 255);
                                    sendto(sockfd, line, 263, 0,
                                           (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                                }
                            }
                        }

                        else
                        {
                            printf("Receiving acknowledgements\n");
                            int temp;
                            memcpy(&temp, &ackLine[0], 4);
                            if (temp < 5)
                            {

                                memcpy(&receivedWindowCounter, &ackLine[0], 4);
                                memcpy(&receivedSeqCount, &ackLine[4], 4);

                                /*if the senderwindows sequence is equal to the received sequence,
                                 * reset the sender window to an empty string
                                 * */
                                if (senderWindow[receivedWindowCounter] == receivedSeqCount)
                                {
                                    senderWindow[receivedWindowCounter] = -1;
                                    char *thing;
                                    thing = "";
                                    memcpy(&windowValue[receivedWindowCounter][0], thing, 255);
                                    printf("Return window value : %d\nReturned sequence number : %d\n\n", receivedWindowCounter, receivedSeqCount);
                                }
                            }
                        }
                    }
                } while (!feof(file) || (senderWindow[0] != -1 || senderWindow[1] != -1 || senderWindow[2] != -1 ||
                                         senderWindow[3] != -1 || senderWindow[4] != -1)); // reached the end of the file

                do
                {
                    char line[263] = "";
                    int eof = -2;
                    int returnedInt = 0;
                    memcpy(&line[4], &eof, 4);
                    sendto(sockfd, line, 263, 0,
                           (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                    recvfrom(sockfd, line, 263, 0,
                             (struct sockaddr *)&clientaddr, &len);
                    memcpy(&returnedInt, &line[4], 4);
                    if (returnedInt == -2)
                    {
                        running = false;
                        break;
                    }
                } while (running);
                fclose(file);
            }
        }
    }
    close(sockfd);
}