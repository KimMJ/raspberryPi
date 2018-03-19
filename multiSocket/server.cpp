#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUFSIZE 100

void* clnt_connection(void *arg);
void send_message(char *message, int len, int sender_fd);
void error_handling(char *message);

int clnt_number = 0;
int clnt_socks[10];
socklen_t clnt_addr_sizes[10];

pthread_mutex_t mutx;

int main(int argc, char **argv){
  int serv_sock, clnt_sock;
  struct sockaddr_in serv_addr;
  struct sockaddr_in clnt_addr;
  int clnt_addr_size;
  pthread_t thread;
  
  if (argc != 2){
    printf("Usage : %s <port>\n", argv[0]);
    exit(1);
  }

  if (pthread_mutex_init(&mutx, NULL)){
    error_handling(const_cast<char *>("mutex init error"));
  }

  serv_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (serv_sock == -1){
    error_handling(const_cast<char *>("socket() error"));
  }

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(atoi(argv[1]));

  if (bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1){
    error_handling(const_cast<char *>("bind() error"));
  }

  if (listen(serv_sock, 5) == -1){
    error_handling(const_cast<char *>("listen() error"));
  }

  while(true){
    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr *) &clnt_addr, (socklen_t *) &clnt_addr_size);
    printf("clnt addr size : %d\n", clnt_addr_size); 

    pthread_mutex_lock(&mutx);
    clnt_addr_sizes[clnt_number] = clnt_addr_size;
    clnt_socks[clnt_number++] = clnt_sock;
    
    pthread_mutex_unlock(&mutx);

    pthread_create(&thread, NULL, clnt_connection, (void *) &clnt_sock);
    printf(" IP : %s \n", inet_ntoa(clnt_addr.sin_addr));
  }
  return 0;
} 

void *clnt_connection(void *arg){
  int clnt_sock = *(int *) arg;
  int str_len = 0;
  char message[BUFSIZE];
  int i;

  while((str_len = read(clnt_sock, message, sizeof(message))) != 0){
    struct sockaddr_in peer_addr;
    getpeername(clnt_sock, (struct sockaddr *) &peer_addr, clnt_addr_sizes + 0); // must change!!
    send_message(message, str_len, clnt_sock);
    printf("peer name : %s\n", inet_ntoa(peer_addr.sin_addr));
  }

  pthread_mutex_lock(&mutx);
  for (i = 0; i < clnt_number; i ++){
    if (clnt_sock == clnt_socks[i]){
      for (; i < clnt_number - 1; i ++){
        clnt_socks[i] = clnt_socks[i+1];
        clnt_addr_sizes[i] = clnt_addr_sizes[i+1];
      }
      break;
    }
  }
  clnt_number --;

  pthread_mutex_unlock(&mutx);

  close(clnt_sock);
  return 0;
}

void send_message(char *message, int len, int sender_fd){
  int i;
  pthread_mutex_lock(&mutx);
  struct sockaddr_in sender_addr, rcv_addr;
  getpeername(sender_fd, (struct sockaddr *) &sender_addr, clnt_addr_sizes + 0);
  for (i = 0; i < clnt_number; i ++){
    getpeername(clnt_socks[i], (struct sockaddr *) &rcv_addr, clnt_addr_sizes + 0);
    printf("sender : %s , receiver : %s\n", inet_ntoa(sender_addr.sin_addr), inet_ntoa(rcv_addr.sin_addr));
    if (strcmp(inet_ntoa(sender_addr.sin_addr), inet_ntoa(rcv_addr.sin_addr)) != 0){
      write(clnt_socks[i], message, len);
    }
  }

  pthread_mutex_unlock(&mutx);
}

void error_handling(char *message){
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}
