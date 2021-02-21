#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include<unistd.h>


//This function is to be used once we have confirmed that an image is to be sent
//It should read and output an image file

int receive_image(int socket)
{ 
  int recv_size = 0, read_size, flag = 0;
  char *filename = "capture.png", imagearray[10241], few_bytes[10241];
  FILE *image;

  char* message = "GET /images/branding/googlelogo/1x/googlelogo_color_272x92dp.png HTTP/1.0\r\nHost: www.google.com\r\n\r\n";

  if (send(socket, message , strlen(message) , 0) < 0) {
    puts("Send failed");
    return 1;
  }

  remove(filename);
  image = fopen(filename, "wb");

  if (image == NULL) {
    printf("Error has occurred. Image file could not be opened\n");
    return -1;
  }

  int cnt = 0, ind = 0;

  do {


    if(flag == 0) {
      flag = 1;
      read_size = read(socket, imagearray, 10241);

      for(int i = 0; i < read_size; i++) {
        if(imagearray[i] == '\n')
          cnt++;

          if(cnt == 13)
          {
            ind = i+1;
            break;
          }
      }

      cnt = 0;

      for(int i = ind; i < read_size; i++) 
        few_bytes[cnt++] = imagearray[ind++]; 
      
      few_bytes[cnt] = '\0';
      fwrite(few_bytes, 1, cnt, image);
      recv_size += read_size;
      continue;
    }
  
    read_size = read(socket, imagearray, 10241);
    printf("Packet size: %i\n", read_size);
    puts(imagearray);
    fwrite(imagearray, 1, read_size, image);
    recv_size += read_size;
    printf("Total received image size: %i\n", recv_size);
   }while(read_size > 0);

  fclose(image);
  printf("Image successfully Received!\n");
  return 1;
}








int main(int argc , char *argv[])
{

  int socket_desc;
  struct sockaddr_in server;
  char *parray;


  //Create socket
  socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  if (socket_desc == -1)
  {
    printf("Could not create socket");
  }

  server.sin_addr.s_addr = inet_addr("142.250.80.14");
  server.sin_family = AF_INET;
  server.sin_port = htons( 80 );

  //Connect to remote server
  if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
  {
    puts("connect error");
    return 1;
  }

  puts("Connected\n");
  receive_image(socket_desc);
  close(socket_desc);

  return 0;
}