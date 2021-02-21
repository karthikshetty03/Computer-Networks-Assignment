#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include<unistd.h>

//download logo
int receive_image(int socket)
{
  int recv_size = 0, read_size, flag = 0, cnt = 0, in = 0;
  //char *filename = "capture.png";
  char *filename = "capture.jpeg";
  char imagearray[10241], few_bytes[10241];
  FILE *image;

  //send request to remote server
  char *message = "GET /v1/images/shopdisney-logo-desktop_1f595224.jpeg?region=0,0,1536,300 HTTP/1.0\r\nHost: lumiere-a.akamaihd.net\r\n\r\n";
  /*
  char* message = "GET /images/branding/googlelogo/1x/googlelogo_color_272x92dp.png HTTP/1.0\r\nHost: www.google.com\r\n\r\n";
  */
  if (send(socket, message , strlen(message) , 0) < 0) {
    puts("Send failed");
    return 1;
  }

  remove(filename);
  image = fopen(filename, "w");

  if (image == NULL) {
    printf("Error has occurred. Image file could not be opened\n");
    return -1;
  }

  //start recieving image data
  do {
    //first chunk of response, so need to separate the headers
    if (flag == 0) {
      flag = 1;
      read_size = read(socket, imagearray, 10241);

      for(int i = 0; i < read_size; i++) {
          if(in == 0 && imagearray[i] != '\r')
              continue;
          else if(in == 0 && imagearray[i] == '\r')
          {
              if(imagearray[i+1] == '\n' && imagearray[i+2] == '\r' && imagearray[i+3] == '\n')
              {
                  in = 1;
                  i += 3;
              }
          }
          else if(in == 1)
              few_bytes[cnt++] = imagearray[i];
      }
      
      few_bytes[cnt] = '\0';
      fwrite(few_bytes, 1, cnt, image);
      recv_size += read_size;
      continue;
    }

    //subsequent chunks of data
    read_size = read(socket, imagearray, 10241);
    fwrite(imagearray, 1, read_size, image);
    recv_size += read_size;
  } while (read_size > 0);

  fclose(image);
  puts("Image successfully Received!");
  return 1;
}

//download website
int recieve_website(int socket) {

}







//driver code
int main(int argc, char *argv[])
{
  int socket_desc;
  struct sockaddr_in server;

  socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  if (socket_desc == -1)
    printf("Could not create socket");

  //server.sin_addr.s_addr = inet_addr("142.250.80.14");
  server.sin_addr.s_addr = inet_addr("104.126.116.211");
  server.sin_family = AF_INET;
  server.sin_port = htons( 80 );

  if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
  {
    puts("connect error");
    return 1;
  }

  puts("Connected");
  receive_image(socket_desc);
  close(socket_desc);

  return 0;
}