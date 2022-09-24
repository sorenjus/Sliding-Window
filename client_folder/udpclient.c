#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>

int main(int argc, char **argv)
{
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
  {
    printf("There was an error creating the socket\n");
    return 1;
  }

  char addrIP[20] = "";
  int portNum = 9876;
  int windowCounter = 0, acknowledgementsSent = 0;
  int receivingWindow[5] = {0, 0, 0, 0, 0};
  int acknowledgements[6] = {0, 0, 0, 0, 0, 0};
  int senderWindow[5], senderReceipt[6];
  // Holds server response
  char serverResponse[255];
  // Holds the file name for writing
  char *filename;
  // FILE file to write contents to
  FILE *file;
  // Setting up IP, Socket information, and port number
  // printf("Enter an IP Address : \n");
  // fgets(addrIP,5000,stdin);
  // printf("Enter a port number :");
  // scanf("%d%*c", &portNum);
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
    recvfrom(sockfd, serverResponse, 255, 0,
             (struct sockaddr *)&serveraddr, &len);
    printf("Server Response: %s\n", serverResponse);
    fputs(serverResponse, file);
    recvfrom(sockfd, senderReceipt, 6, 0,
             (struct sockaddr *)&serveraddr, &len);
    printf("Received packet\n");
    // Now we have the sender receipt array
    // If the sender receipt matches the order we were expecting
    if (windowCounter == senderReceipt[5] && senderReceipt[windowCounter] == 1)
    {
      printf("Window Counter: %d\n", windowCounter);
      printf("Sender Receipt: %d\n", senderReceipt[5]);
      printf("Sender receipt at counter: %d\n", senderReceipt[windowCounter]);
      receivingWindow[windowCounter] = 1;
      acknowledgements[windowCounter] = 1;
      // This array is of length 6.  0-4 for the acknowledgements and the 5th
      // spot for the index that was just acknowledged.
      acknowledgements[5] = windowCounter;
      sendto(sockfd, acknowledgements, 6, 0,
             (struct sockaddr *)&serveraddr, sizeof(serveraddr));
      acknowledgementsSent++;
      windowCounter++;
    }
    // what happens when the sender sends a packet in a different order
    else
    {
      printf("Window Counter: %d\n", windowCounter);
      printf("Sender Receipt: %d\n", senderReceipt[5]);
      printf("Sender receipt at counter: %d\n", senderReceipt[windowCounter]);

      windowCounter = senderReceipt[5];
      receivingWindow[windowCounter] = 1;
      acknowledgements[windowCounter] = 1;
      acknowledgements[5] = windowCounter;
      sendto(sockfd, acknowledgements, 6, 0,
             (struct sockaddr *)&serveraddr, sizeof(serveraddr));
      acknowledgementsSent++;
      windowCounter++;
    }
    // windowCounter++;

  } while (windowCounter < 5);

  fclose(file);
  close(sockfd);

  return 0;
}
