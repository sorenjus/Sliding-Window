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
  //  // if they can receive more packets
  //  int receivingWindow[5] = {0, 0, 0, 0, 0};
  //  int acknowledgements[6] = {0, 0, 0, 0, 0, 0};
  //  int senderReceipt[6];
  int fileSequence = 0;
  int tempSequence = 0;
  int nextPacket = 0;
  // Holds server response
  char serverResponse[5][255];

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

  if ((file = fopen(filename, "w")) == NULL)
  {
    fprintf(stderr, "Cannot write to output file");
    return 1;
  }

  // Send to server
  sendto(sockfd, &userInput, strlen(userInput), 0,
         (struct sockaddr *)&serveraddr, sizeof(serveraddr));
  int seqArray[5] = {-1, -1, -1, -1, -1};
  do
  {
    /*add a time out here. If it times out, resend all acks?
     */
    char line[264] = "";
    // receive the packet from the server
    int t = recvfrom(sockfd, line, 255, 0,
                     (struct sockaddr *)&serveraddr, &len);
    if (t == -1)
    {
      if (errno == EWOULDBLOCK)
      {
        printf("Timed out while waiting for server\n");
        for (int i = 0; i < 5; ++i)
        {
          if (seqArray[i] != -1)
          {
            char ackLine[9] = "";
            memcpy(&ackLine[0], &windowCounter, 4);
            memcpy(&ackLine[4], &fileSequence, 4);
            sendto(sockfd, ackLine, 8, 0,
                   (struct sockaddr *)&serveraddr, sizeof(serveraddr));

            printf("Sent acknowledgement\nWindow : %d\nCurrent sequence : %d\n\n", windowCounter, fileSequence);
          }
        }
      }
    }
    else
    {

      if (strstr(line, "EOF"))
      {
        char *str = "EOF";
        sendto(sockfd, str, 263, 0,
               (struct sockaddr *)&serveraddr, sizeof(clientaddr));
        break;
      }

      memcpy(&windowCounter, &line[0], 4);
      memcpy(&tempSequence, &line[4], 4);
      /* copy this to a temp variable first and compare it to the current held sequence
      If greater, copy and continue. Else ignore
      */
      if (tempSequence > fileSequence)
      {
        memcpy(&fileSequence, &line[4], 4);
      }
      strcpy(serverResponse[windowCounter], &line[8]);
      seqArray[windowCounter] = fileSequence;

      // print the packet contents
      printf("Received packet\n");
      printf("Server Response: %s\n", serverResponse[windowCounter]);
      printf("Received at window : %d\n", windowCounter);
      printf("Sequence count : %d\n", fileSequence);

      // Check if the file steam has ended
      if (!strcmp(serverResponse[windowCounter], "EOF"))
      {
        printf("Running now false\n");
        running = false;
      }
      else
      {
        // if the packet is the next packet in the sequence, add it to the file and send acknowledgement
        if (fileSequence == nextPacket)
        {
          printf("Adding packet contents to file\n\n");
          // add the response to the file
          fputs(serverResponse[windowCounter], file);
          nextPacket++;

          char ackLine[9] = "";
          memcpy(&ackLine[0], &windowCounter, 4);
          memcpy(&ackLine[4], &fileSequence, 4);
          sendto(sockfd, ackLine, 8, 0,
                 (struct sockaddr *)&serveraddr, sizeof(serveraddr));

          printf("Sent acknowledgement\nWindow : %d\nCurrent sequence : %d\n\n", windowCounter, fileSequence);
        }
        else
        {
          for (int i = 0; i < 5; ++i)
          {
            if (seqArray[i] == nextPacket)
            {
              fputs(serverResponse[i], file);
              char ackLine[9] = "";
              memcpy(&ackLine[0], &i, 4);
              memcpy(&ackLine[4], &seqArray[i], 4);
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
