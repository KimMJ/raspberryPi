#include "server.h"

struct epoll_event g_events[MAX_EVENTS];
struct client g_clients[MAX_CLIENT];
int g_epoll_fd;


int main(int argc, char **argv){
  int server_socket;
  struct sockaddr_in server_address;
  if (argc != 2){
    printf("Usage : %s <port>\n", argv[0]);
    exit(1);
  } else if (atoi(argv[1]) < 1024){
    error_handling("invalid port number\n");
  }

  for (int i = 0; i < MAX_CLIENT; i ++){
    g_clients[i].client_socket_fd = -1;
  }
/*
  if (pthread_mutex_init(&mutx, NULL)){
    error_handling(const_cast<char *>("mutex init error\n"));
  }
*/
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == server_socket){
    error_handling(const_cast<char *>("socket() error\n"));
  }

  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(atoi(argv[1]));

  int n_socket_opt = 1;
  if( setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &n_socket_opt, sizeof(n_socket_opt)) < 0 ){
    printf("Server Start Fails. : Can't set reuse address\n");
    close(server_socket);
    exit(1);
  }

  if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) == -1){
    printf("bind() error\n");
    close(server_socket);
    exit(1);
  }

  if (listen(server_socket, 15) == -1){
    printf("listen() error\n");
    close(server_socket);
    exit(1);
  }

  printf("server start listening PORT = %d\n", atoi(argv[1]));

  //epoll init
  struct epoll_event events;

  g_epoll_fd = epoll_create(MAX_EVENTS);
  if (g_epoll_fd < 0){
    printf("epoll create fails\n");
    close(server_socket);
    exit(1);
  }

  printf("epoll created\n");

  events.events = EPOLLIN;
  events.data.fd = server_socket;

  if (epoll_ctl(g_epoll_fd, EPOLL_CTL_ADD, server_socket, &events) < 0){
    printf("epoll control failed\n");
    close(server_socket);
    exit(1);
  }

  printf("epoll set succeded\n");

  while (true){
    struct sockaddr_in client_address;
    int client_length = sizeof(client_address);
    int client_socket;
    int num_fd = epoll_wait(g_epoll_fd, g_events, MAX_EVENTS, 100);
    if (num_fd == 0){
      continue; // nothing
    } else if (num_fd < 0){
      printf("epoll wait error : %d\n",num_fd );
      for (int i = 0; i < MAX_EVENTS; i++){
        if (g_clients[i].client_socket_fd > 0){
          printf("g_client[i].client_socket_fd = %d\n", g_clients[i].client_socket_fd);
        
        }
      }
      printf("%s\n",g_events[0].events);
      continue;
    }

    for (int i = 0; i < num_fd; i ++){
      if (g_events[i].data.fd == server_socket){
        client_socket = accept(server_socket, (struct sockaddr *) &client_address, (socklen_t *) &client_length);
        if (client_socket < 0){
          printf("accept_error\n");
        } else {
          printf("new client connected\nfd : %d\nip : %d\n", client_socket, inet_ntoa(client_address.sin_addr));
          userpoll_add(client_socket, inet_ntoa(client_address.sin_addr));
        }
      } else {
        client_receive(g_events[i].data.fd);
      }
    }
  }
  
}

void error_handling(char *message){
  fputs(message, stderr);
  exit(1);
}

void userpoll_add(int client_fd, char * client_ip){
  int i;
  for (i = 0; i < MAX_CLIENT; i ++){
    if (g_clients[i].client_socket_fd == -1) break;
  }
  if (i >= MAX_CLIENT) {
    printf("client is full\n");
    close(client_fd);
  }

  g_clients[i].client_socket_fd = client_fd;
  memset(&g_clients[i].client_ip[0], 0, 20);
  strcpy(&g_clients[i].client_ip[0], client_ip);

  struct epoll_event events;

  events.events = EPOLLIN;
  events.data.fd = client_fd;

  if (epoll_ctl(g_epoll_fd, EPOLL_CTL_ADD, client_fd, &events) < 0){
    printf("epoll control failed for client\n");
  }
}

void userpoll_delete(int client_fd){
  int i;
  for (int i = 0; i < MAX_CLIENT; i ++){
    if(g_clients[i].client_socket_fd == client_fd){
      g_clients[i].client_socket_fd = -1;
      break;
    }
  }
}

void client_receive(int event_fd){
  char buf[BUFSIZE];
  int len;
  len = recv(event_fd, buf, BUFSIZE, 0);
  if (len <= 0){
    userpoll_delete(event_fd);
    close(event_fd);
    return;
  }

  for (int i = 0; i < MAX_CLIENT; i ++){
    if (g_clients[i].client_socket_fd != -1){
      len = send(g_clients[i].client_socket_fd, buf, len, 0);
    }
  }
}

