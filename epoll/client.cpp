#include "client.hpp"
#include <opencv/cv.hpp>
#include <unistd.h>
#include <string>
#include <iostream>

using namespace std;
using namespace cv;

#define INPUT "../cctv.avi"
#define OUTPUT_PREFIX "output"
#define OUTPUT_POSTFIX ".jpg"

bool socket_connected = false;

int main(int argc, char **argv){
  int sock;
  struct sockaddr_in server_address;
  pthread_t snd_thread, rcv_thread;
  void *thread_result;

  if (!(argc == 3)){
    printf("Usage : %s <ip> <port> \n", argv[0]);
    exit(1);
  }

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    printf("socket() error\n");
    exit(1);
  }

  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = inet_addr(argv[1]);
  server_address.sin_port = htons(atoi(argv[2]));

  if (connect(sock, (struct sockaddr *) &server_address, sizeof(server_address)) == -1){
    printf("connect() error\n");
    exit(1);
  }
  socket_connected = true;

  pthread_create(&snd_thread, NULL, send_data, (void *) &sock);
  pthread_create(&rcv_thread, NULL, recv_data, (void *) &sock);

  pthread_join(snd_thread, &thread_result);
  pthread_join(rcv_thread, &thread_result);
  
  close(sock);
  return 0;
}

void *send_data(void *arg){
  int sock = *(int *) arg;
  char data[BUFSIZE] = {0};
  int fd;

  // OPENCV
  int iCurrentFrame = 0;
  VideoCapture vc = VideoCapture(INPUT);
  if (!vc.isOpened()) {
      cerr << "fail to open the video" << endl;
      return (void*)EXIT_FAILURE;
  }

  while (socket_connected == true) {
    double fps = vc.get(CV_CAP_PROP_FPS);
    cout << "total : " << vc.get(CV_CAP_PROP_FRAME_COUNT) << endl;

    while (true) {
        //vc.set(CV_CAP_PROP_POS_FRAMES, iCurrentFrame);
        int posFrame = vc.get(CV_CAP_PROP_POS_FRAMES);

        Mat frame;
        vc >> frame;

        if (frame.empty()) {
          exit(0);
            return (void*) 0;
        }

        if (posFrame % 100 == 0) { // every 500 frames
          imshow("image", frame);
          stringstream ss;
          ss << "";
          string filename = OUTPUT_PREFIX + ss.str() + OUTPUT_POSTFIX;
          imwrite(filename.c_str(), frame);
          break;
        }
        waitKey(1);
    }
    // END OF OPENCV

    //fgets(data, BUFSIZE, stdin);

    if (!strcmp(data, "q\n")){
      close(sock);
      exit(0);
    } else /*if (!strcmp(data, "transfer\n"))*/ {
      FILE *f;
      f = fopen("output.jpg" , "r");
      fseek(f, 0, SEEK_END);
      unsigned long file_len = (unsigned long)ftell(f);

      fd = open("output.jpg", O_RDONLY);
      
      if (fd == -1) {
        printf("no file\n");
        exit(1);
      }

      //notice transfer
      sprintf(data, "%u\0", file_len);
      printf("%u\0", file_len);
      write(sock, data, strlen(data));
      usleep(100);

      int len = 0;
      while ((len=read(fd, data, BUFSIZE)) != 0) {
        //puts(data);
        printf("transferring len : %d\n", len);
        write(sock, data, len);    
      }
      printf("done!\n");
    } /*else {
      sprintf(data, "%s", data);
      write(sock, data, strlen(data));
    }*/
  }
}

void *recv_data(void *arg){
  int sock = *(int *) arg;
  char data[BUFSIZE];
  int str_len;
  while (true){
    str_len = read(sock, data, BUFSIZE - 1);
    if (str_len == -1){
      return (void *) 1;
    }
    if (str_len == 0){
      printf("socket closed\n");
      socket_connected = false;
      return (void *) 1;
    }
    data[str_len] = 0;
    fputs(data, stdout);
  }
}
