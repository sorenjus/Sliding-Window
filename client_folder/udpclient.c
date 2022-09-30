#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

bool running = true;

int main(int argc, char **argv)
{
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
  {
    printf("There was an error creating the socket\n");
    return 1;
  }

  int portNum = 9876;

  int windowCounter = 0, acknowledgementsSent = 0;
  //if they can receive more packets
  int receivingWindow[5] = {0, 0, 0, 0, 0};
  int acknowledgements[6] = {0, 0, 0, 0, 0, 0};
  //int senderReceipt[6];
  int totalCountReceived = 0;
  int nextPacket = 0;
  // Holds server response
  char serverResponse[5][255];

  // Holds the file name for writing
  char *filename;
  // FILE file to write contents to
  FILE *file;
  struct sockaddr_in serveraddr, clientaddr;
  socklen_t len = sizeof(clientaddr);
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(portNum);
  // serveraddr.sin_addr.s_addr=inet_addr(addrIP);
  serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Asking user for a filename and scanning
  printf("Enter a filename to retrieve: \n");
  char userInput[255];
  scanf("%s", userInput);
  printf("Retrieving %s...\n", userInput);
  filename = userInput;

  if ((file = fopen(filename, "w")) == NULL)
  {
    fprintf(stderr, "Cannot write to output file");
    return 1;
  }

  // Send to server
  sendto(sockfd, &userInput, strlen(userInput), 0,
         (struct sockaddr *)&serveraddr, sizeof(serveraddr));

  do
  {
    char line[264] = "";
    // To do: Check for missing packets before fputs.
    // To do: fflush
    // To do: figure out how to stop
    /*******************************Not totally sure how this works*********************
    if (acknowledgementsSent == 5 && windowCounter == 5)
    {
      windowCounter = 0;
      acknowledgementsSent = 0;
      memset(acknowledgements, 0, 6 * sizeof(int));
      memset(receivingWindow, 0, 5 * sizeof(int));
    }
    */
   //receive the packet from the server
   printf("bus error3?");
    recvfrom(sockfd, line, 255, 0,
             (struct sockaddr *)&serveraddr, &len);
    memcpy(&windowCounter, &line[0], 4);
    memcpy(&totalCountReceived, &line[4], 4);
    strcpy(serverResponse[windowCounter], &line[8]);
    //print the packet contents
    printf("Received packet\n");
    printf("Server Response: %s\n", serverResponse[windowCounter]);
    printf("Received at window : %d\n", windowCounter);
    printf("Sequence count : %d\n", totalCountReceived);

    //Check if the file steam has ended
    if (!strcmp(serverResponse[windowCounter], "EOF"))
    {
      printf("Running now false\n");
      running = false;
    }
    else
    {
      //if the packet is the next packet in the sequence, add it to the file and send acknowledgement 
      if(totalCountReceived == nextPacket){
      printf("Adding packet contents to file\n\n");
      //add the response to the file
      fputs(serverResponse[windowCounter], file);
      nextPacket++;

      char ackLine[8];
      memcpy(&ackLine[0], &windowCounter, 4);
      memcpy(&ackLine[4], &totalCountReceived, 4);
      sendto(sockfd, &ackLine, 8, 0,
         (struct sockaddr *)&serveraddr, sizeof(serveraddr));
      printf("Sent acknowledgement\nWindow : %d\nCurrent sequence : %d\n\n", windowCounter, totalCountReceived);
      printf("bus error1?");
      }
      // Now we have the sender receipt array
      // If the sender receipt matches the order we were expecting
      /******************************Not totally sure how this works*********************
      if (windowCounter == senderReceipt[5] && senderReceipt[windowCounter] == 1)
      {
        printf("Window Counter: %d\n", windowCounter);
        printf("Sender Receipt for index 5: %d\n", senderReceipt[5]);
        printf("Sender receipt at counter: %d\n", senderReceipt[windowCounter]);
        receivingWindow[windowCounter] = 1;
        acknowledgements[windowCounter] = 1;
        // This array is of length 6.  0-4 for the acknowledgements and the 5th
        // spot for the index that was just acknowledged.
        acknowledgements[5] = windowCounter;
        sendto(sockfd, acknowledgements, 24, 0,
               (struct sockaddr *)&serveraddr, sizeof(serveraddr));
        acknowledgementsSent++;
        windowCounter++;
      }
      */

        //currently sender receipt does not exist
        //printf("Window Counter: %d\n", windowCounter);
        //printf("Sender Receipt: %d\n", senderReceipt[5]);
        //printf("Sender receipt at counter: %d\n", senderReceipt[windowCounter]);

        //windowCounter = senderReceipt[5];
        /*receivingWindow[windowCounter] = 1;
        acknowledgements[windowCounter] = 1;
        acknowledgements[5] = windowCounter;
        sendto(sockfd, acknowledgements, 24, 0,
               (struct sockaddr *)&serveraddr, sizeof(serveraddr));
        acknowledgementsSent++;
        windowCounter++;
    */
      // windowCounter++;
      printf("bus error2?");
    }
  } while (running);

  fclose(file);
  close(sockfd);

  return 0;
}
