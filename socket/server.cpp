#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

void error(char *msg){
  perror(msg);
  exit(1);
}

int func (int a){
  return 2 * a;
}

void sendData(int sockfd, int x){
  int n;
  char buffer [32];
  sprintf(buffer, "%d\n", x);
  if ((n = write(sockfd, buffer, strlen(buffer))) < 0){
    error(const_cast<char *>("ERROR writing to socket"));
  }
  buffer[n] = '\0';
}

int getData(int sockfd){
  char buffer[32];
  int n;

  if ((n = read(sockfd, buffer, 31)) < 0){
    error(const_cast<char *>("ERROR reading from socket"));
  }
  buffer[n] = '\0';
  return atoi(buffer);
}

int main (int argc, char *argv[]){
  int sockfd, newsockfd, portno = 51717, clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n;
  int data;

  printf("using port #%d\n", portno);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0){
    error(const_cast<char *>("ERROR opening socket"));
  }
  bzero((char *) &serv_addr, sizeof(serv_addr)); //change to memset

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    error(const_cast<char *>("ERROR on binding"));
  }
  listen(sockfd, 5);
  clilen = sizeof(cli_addr);

  while (true){
    printf("waiting for new client\n");
    if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *) &clilen)) < 0){
      error(const_cast<char *>("ERROR on accept"));
    }
    printf("opened new communication with client\n");
    while (true){
      data = getData(newsockfd);
      printf("got %d\n", data);
      if (data < 0){
        break;
      }
      data = func(data);

      printf("sending back %d\n", data);
      sendData(newsockfd, data);
    }
    close(newsockfd);
    if (data == -2){
      break;
    }
  }


  return 0;
}
