#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <errno.h>

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
  struct timeval timeout;
  timeout.tv_sec = 5;
  timeout.tv_usec = 0;
  int windowCounter = 0;
  // acknowledgementsSent = 0;
  //Hold the current sequence number
  int receivingWindow[5] = {0, 0, 0, 0, 0};
  char windowValue[5][255];
  //  int acknowledgements[6] = {0, 0, 0, 0, 0, 0};
  //  int senderReceipt[6];
  int nextPacket = 0;
  // Holds server response
  

  // Holds the file name for writing
  char *filename;
  // FILE file to write contents to
  FILE *file;

  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
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
  int tempSequence = 0;

  if ((file = fopen(filename, "w")) == NULL)
  {
    fprintf(stderr, "Cannot write to output file");
    return 1;
  }

  // Send to server
  sendto(sockfd, &userInput, strlen(userInput), 0,
         (struct sockaddr *)&serveraddr, sizeof(serveraddr));

  int timeoutCounter = 0;
  do
  {
    char line[264] = "";
    // receive the packet from the server
    int t = recvfrom(sockfd, line, 255, 0,
                     (struct sockaddr *)&serveraddr, &len);
    // Determine if the server could not open file
    if (strstr(line, "Error!"))
    {
      printf("Server could not find file.\n");
      fclose(file);
      if (remove(filename) != 0)
      {
        printf("Error deleting file\n");
      }
      close(sockfd);
      return 0;
    }
    if (t == -1)
    {
      if (errno == EWOULDBLOCK)
      {
        printf("Timed out while waiting for server\n");
        for (int i = 0; i < 5; ++i)
        {
            char ackLine[9] = "";
            memcpy(&ackLine[0], &i, 4);
            memcpy(&ackLine[4], &receivingWindow[i], 4);
            sendto(sockfd, ackLine, 8, 0,
                   (struct sockaddr *)&serveraddr, sizeof(serveraddr));

            printf("Sent acknowledgement\nWindow : %d\nCurrent sequence : %d\n\n", windowCounter, receivingWindow[windowCounter]);
          }
        }
      }
    else
    {
      if (strstr(line, "EOF"))
      {
        running = false;
        char *str = "EOF";
        sendto(sockfd, str, 263, 0,
               (struct sockaddr *)&serveraddr, sizeof(clientaddr));
        break;
      }

      memcpy(&windowCounter, &line[0], 4);
      memcpy(&tempSequence, &line[4], 4);

      if (tempSequence > receivingWindow[windowCounter])
      {
        memcpy(&receivingWindow[windowCounter], &line[4], 4);
      }
      strcpy(windowValue[windowCounter], &line[8]);

      // print the packet contents
      printf("Received packet\n");
      printf("Server Response: %s\n", windowValue[windowCounter]);
      printf("Received at window : %d\n", windowCounter);
      printf("Sequence count : %d\n", receivingWindow[windowCounter]);

      // Check if the file steam has ended
      if (!strcmp(windowValue[windowCounter], "EOF"))
      {
        printf("Running now false\n");
        running = false;
      }
      else
      {
        // if the packet is the next packet in the sequence, add it to the file and send acknowledgement
        if (receivingWindow[windowCounter] == nextPacket)
        {
          printf("Adding packet contents to file\n\n");
          // add the response to the file
          fputs(windowValue[windowCounter], file);
          nextPacket++;

          char ackLine[9] = "";
          memcpy(&ackLine[0], &windowCounter, 4);
          memcpy(&ackLine[4], &receivingWindow[windowCounter], 4);
          sendto(sockfd, ackLine, 8, 0,
                 (struct sockaddr *)&serveraddr, sizeof(serveraddr));

          printf("Sent acknowledgement\nWindow : %d\nCurrent sequence : %d\n\n", windowCounter, receivingWindow[windowCounter]);
        }
        else
        {
          for (int i = 0; i < 5; ++i)
          {
            if (receivingWindow[i] == nextPacket)
            {
              fputs(windowValue[i], file);
              nextPacket++;
              char ackLine[9] = "";
              memcpy(&ackLine[0], &i, 4);
              memcpy(&ackLine[4], &receivingWindow[i], 4);
              sendto(sockfd, ackLine, 8, 0,
                     (struct sockaddr *)&serveraddr, sizeof(serveraddr));
            }
          }
        }
      }
    }
  } while (running);

  fclose(file);
  close(sockfd);

  return 0;
}
