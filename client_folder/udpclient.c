#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

// Authors : Justin Sorensen, Meghan Harris

bool running = true;

int main(int argc, char **argv)
{
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
  {
    printf("There was an error creating the socket\n");
    return 1;
  }

  int windowCounter = 0;
  int receivingWindow[5] = {-1, -1, -1, -1, -1};
  int bufferWindow[5] = {-1, -1, -1, -1, -1};
  char windowValue[5][255];
  char bufferValue[5][255];
  int tempSequence = 0;

  int nextPacket = 0;
  // Holds the file name for writing
  char *filename;
  // FILE to write contents to
  FILE *file;

  char addrIP[20] = "";
  int portNum = 9876;
  printf("Enter an IP Address :");
  fgets(addrIP, 5000, stdin);
  printf("Enter a port number :");
  scanf("%d%*c", &portNum);

  struct timeval timeout;
  timeout.tv_sec = 5;
  timeout.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  struct sockaddr_in serveraddr, clientaddr;
  socklen_t len = sizeof(clientaddr);
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(portNum);
  serveraddr.sin_addr.s_addr = inet_addr(addrIP);

  // Asking user for a filename and scanning
  printf("Enter a filename to retrieve: \n");
  char userInput[50];
  scanf("%49s", userInput);
  printf("Retrieving %s...\n", userInput);
  filename = userInput;

  bool text = strstr(filename, ".txt");

  if (text)
  {
    if ((file = fopen(filename, "w")) == NULL)
    {
      fprintf(stderr, "Cannot write to output file");
      return 1;
    }
  }
  else
  {
    if ((file = fopen(filename, "wb")) == NULL)
    {
      fprintf(stderr, "Cannot write to output file");
      return 1;
    }
  }

  // Send to server
  sendto(sockfd, &userInput, strlen(userInput), 0,
         (struct sockaddr *)&serveraddr, sizeof(serveraddr));

  do
  {
    char line[264] = "";
    // receive the packet from the server
    int t = recvfrom(sockfd, line, 255, 0,
                     (struct sockaddr *)&serveraddr, &len);

    // if no new data has been added, timeout, and first packet has received no data
    if (nextPacket == 0 && t == -1 && receivingWindow[0] == -1 && receivingWindow[1] == -1 && receivingWindow[2] == -1 && receivingWindow[3] == -1 && receivingWindow[4] == -1)
    {
      printf("No response from server. Resending file request\n");
      sendto(sockfd, &userInput, strlen(userInput), 0,
             (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    }
    else
    {
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
          
            printf("Checking window for next dataline\n");
            for (int i = 0; i < 5; ++i)
            {
                nextPacket++;
                char ackLine[263] = "";
                memcpy(&ackLine[0], &i, 4);
                memcpy(&ackLine[4], &receivingWindow[i], 4);
                strcpy(&ackLine[8], &windowValue[i][0]);
                sendto(sockfd, ackLine, 263, 0,
                       (struct sockaddr *)&serveraddr, sizeof(serveraddr));
            }
          
          printf("Timed out while waiting for server\nResending acknowledgements\n");
          for (int i = 0; i < 5; ++i)
          {
            if (receivingWindow[i] == -2)
            {
              printf("timeout. received EOF");
              running = false;
            }
            else if (receivingWindow[i] < nextPacket)
            {
              char ackLine[263] = "";
              memcpy(&ackLine[0], &i, 4);
              memcpy(&ackLine[4], &receivingWindow[i], 4);
              strcpy(&ackLine[8], &windowValue[i][0]);
              sendto(sockfd, ackLine, 263, 0,
                     (struct sockaddr *)&serveraddr, sizeof(serveraddr));

              printf("Sent acknowledgement\nWindow : %d\nCurrent sequence : %d\n\n", i, receivingWindow[i]);
            }
          }
        }
      }
      else
      {
        memcpy(&windowCounter, &line[0], 4);
        memcpy(&tempSequence, &line[4], 4);

        if (tempSequence >= receivingWindow[windowCounter])
        {
          memcpy(&receivingWindow[windowCounter], &line[4], 4);
          strcpy(windowValue[windowCounter], &line[8]);
        }
        else if (tempSequence == (receivingWindow[windowCounter] + 5))
        {
          memcpy(&bufferWindow[windowCounter], &line[4], 4);
          strcpy(bufferValue[windowCounter], &line[8]);

          if (receivingWindow[windowCounter] == nextPacket)
          {
            printf("Adding packet contents to file\n\n");
            // add the response to the file
            if (text)
            {
              fputs(windowValue[windowCounter], file);
            }
            else
            {
              fwrite(windowValue[windowCounter], sizeof(windowValue[windowCounter][0]), 1, file); // write 10 bytes from our buffer
            }
            nextPacket++;

            memcpy(&receivingWindow[windowCounter], &bufferWindow[windowCounter], 4); //sequence
            strcpy(windowValue[windowCounter], bufferValue[windowCounter]); //line
          }
        }
          
          else if (tempSequence == -2)
          {
            printf("Received EOF");
            memcpy(&receivingWindow[windowCounter], &line[4], 4);
            running = false;
            char ackLine[263] = "";
              memcpy(&ackLine[0], &windowCounter, 4);
              memcpy(&ackLine[4], &receivingWindow[windowCounter], 4);
              strcpy(&ackLine[8], &windowValue[windowCounter][0]);
              sendto(sockfd, ackLine, 263, 0,
                     (struct sockaddr *)&serveraddr, sizeof(serveraddr));
          }
          

          // print the packet contents
          printf("Received packet\n");
          printf("Server Response: %s\n", windowValue[windowCounter]);
          printf("Received at window : %d\n", windowCounter);
          printf("Sequence count : %d\n", receivingWindow[windowCounter]);

          char ackLine[263] = "";
          memcpy(&ackLine[0], &windowCounter, 4);
          memcpy(&ackLine[4], &receivingWindow[windowCounter], 4);
          strcpy(&ackLine[8], &windowValue[windowCounter][0]);
          sendto(sockfd, ackLine, 263, 0,
                 (struct sockaddr *)&serveraddr, sizeof(serveraddr));

          // ensure the next file line does not currently exist in the receivingWindow
/*
          for(int j = 0; j < 5; j++){
            printf("Checking window for next dataline\n");
            for (int i = 0; i < 5; ++i)
            {
              if (receivingWindow[i] == nextPacket)
              {
                if (text)
                {
                  fputs(windowValue[i], file);
                }
                else
                {
                  fwrite(windowValue[i], sizeof(windowValue[i][0]), 1, file); // write 10 bytes from our buffer
                }
                nextPacket++;
                char ackLine[263] = "";
                memcpy(&ackLine[0], &i, 4);
              memcpy(&ackLine[4], &receivingWindow[i], 4);
              strcpy(&ackLine[8], &windowValue[i][0]);
              sendto(sockfd, ackLine, 263, 0,
                     (struct sockaddr *)&serveraddr, sizeof(serveraddr));
              }
            }
          }*/
        }
      }
    }
    while (running)
      ;

    // Check the receiving window for any values that have not yet been added to the file
    for (int j = 0; j < 5; j++)
    {
      printf("Final run through\n");
      for (int i = 0; i < 5; ++i)
      {
        if (receivingWindow[i] == nextPacket)
        {
          if (text)
          {
            fputs(windowValue[i], file);
          }
          else
          {
            fwrite(windowValue[i], sizeof(windowValue[i][0]), 1, file); // write 10 bytes from our buffer
          }
          nextPacket++;
          char ackLine[263] = "";
          memcpy(&ackLine[0], &i, 4);
          memcpy(&ackLine[4], &receivingWindow[i], 4);
          strcpy(&ackLine[8], &windowValue[i][0]);
          sendto(sockfd, ackLine, 263, 0,
                 (struct sockaddr *)&serveraddr, sizeof(serveraddr));
        }
      }
    }

    fclose(file);
    close(sockfd);

    return 0;
  }
