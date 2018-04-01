#ifndef _SERVER_H_
#define _SERVER_H_
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENT 1000
#define DEFAULT_PORT 20000
#define MAX_EVENTS 1000
#define BUFSIZE 1024

void error_handling(char *message);
void userpoll_add(int client_fd, char * client_ip);
struct client {
  int client_socket_fd;
  char client_ip[20];
};
void userpoll_delete(int client_fd);
void client_receive(int event_fd);
