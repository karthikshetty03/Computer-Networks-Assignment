#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include <unistd.h>
#include<stdbool.h>

char* get_logo_url(char* filename) {
    char search_string[10241] = "SRC=";
    char word[10241];

    FILE *fptr;
    fptr = fopen(filename, "r");

    while(fscanf(fptr, "%s", word) != EOF) {
        puts(word);
        if(strlen(word) <= 6) {
            continue;
        }

        char req[10241];
        char url[10241];
        int cnt = 0;

        for(int i = 0; i < 4; i++) 
            req[i] = word[i];

        req[4] = '\0';

        if(strcmp(search_string, req) == 0) {
            printf("**************FOUND*****************\n");
            
            for(int i = 5; word[i]!='"'; i++)
                url[cnt++] = word[i];
                
            url[cnt] = '\0';
            puts(url);
            fclose(fptr);
            char* str = (char*)malloc(sizeof(char)*strlen(url));
            strcpy(str, url);
            return str;
        }
    }

    fclose(fptr);
    return "";
}

// Takes string to be encoded as input 
// and its length and returns encoded string 
char* base64Encoder(char input_str[]) 
{ 
    int len_str = strlen(input_str);
	// Character set of base64 encoding scheme 
	char char_set[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; 
	
	// Resultant string 
	char *res_str = (char *) malloc(10241 * sizeof(char)); 
	
	int index, no_of_bits = 0, padding = 0, val = 0, count = 0, temp; 
	int i, j, k = 0; 
	
	// Loop takes 3 characters at a time from 
	// input_str and stores it in val 
	for (i = 0; i < len_str; i += 3) 
		{ 
			val = 0, count = 0, no_of_bits = 0; 

			for (j = i; j < len_str && j <= i + 2; j++) 
			{ 
				// binary data of input_str is stored in val 
				val = val << 8; 
				
				// (A + 0 = A) stores character in val 
				val = val | input_str[j]; 
				
				// calculates how many time loop 
				// ran if "MEN" -> 3 otherwise "ON" -> 2 
				count++; 
			
			} 

			no_of_bits = count * 8; 

			// calculates how many "=" to append after res_str. 
			padding = no_of_bits % 3; 

			// extracts all bits from val (6 at a time) 
			// and find the value of each block 
			while (no_of_bits != 0) 
			{ 
				// retrieve the value of each block 
				if (no_of_bits >= 6) 
				{ 
					temp = no_of_bits - 6; 
					
					// binary of 63 is (111111) f 
					index = (val >> temp) & 63; 
					no_of_bits -= 6;		 
				} 
				else
				{ 
					temp = 6 - no_of_bits; 
					
					// append zeros to right if bits are less than 6 
					index = (val << temp) & 63; 
					no_of_bits = 0; 
				} 
				res_str[k++] = char_set[index]; 
			} 
	} 

	// padding is done here 
	for (i = 1; i <= padding; i++) 
		res_str[k++] = '='; 

	res_str[k] = '\0'; 

	return res_str; 
}

//download logo
int receive_file(int socket, char* filename, char* web_url, char* img_url, char* username, char* password)
{
  int recv_size = 0, read_size, flag = 0, cnt = 0, in = 0;
  char imagearray[10241], few_bytes[10241];
  FILE *image;

  //strip trailing slashes if any
  int len = strlen(web_url);
  while(web_url[len] == '/') {
      web_url[len] = '\0';
      len--;
  }
  
  //encode the credentials
  char raw_auth[1024] = "";
  strcat(raw_auth, username);
  strcat(raw_auth, ":");
  strcat(raw_auth, password);
  
  char *auth_str = base64Encoder(raw_auth);

  //formation of the GET request via. squid proxy
  char message[10241] = "GET http://";
  strcat(message, web_url);
  strcat(message, "/");
  strcat(message,  img_url);
  strcat(message, " HTTP/1.1\r\nHost: ");
  strcat(message, web_url);
  strcat(message, "\r\nProxy-Authorization: Basic ");
  strcat(message, auth_str);
  strcat(message, "\r\nConnection: close\r\n\r\n");

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
          if(in == 0 && imagearray[i] != '\r') {
              continue;
          }
          else if(in == 0 && imagearray[i] == '\r') {
              if(imagearray[i+1] == '\n' && imagearray[i+2] == '\r' && imagearray[i+3] == '\n') {
                  in = 1;
                  i += 3;
              }
          }
          else if(in == 1) {
              few_bytes[cnt++] = imagearray[i];
          }
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
  
  puts("Data successfully Received!");
  return 1;
}

//driver code
int main(int argc, char *argv[]) {
  bool downloadLogo = false;
  char* web_url = "";
  char *proxy_ip = "";
  char *proxy_port = "";
  char* username = "";
  char* password = "";
  char* file_name = "";
  char* img_name = "";

  web_url = argv[1];
  proxy_ip = argv[2];
  proxy_port = argv[3];
  username = argv[4];
  password = argv[5];
  file_name = argv[6];

  if(argc == 8) {
    downloadLogo = true;
    img_name = argv[7];
  }

  int socket_desc;

  struct sockaddr_in server;
  socket_desc = socket(AF_INET , SOCK_STREAM , 0);

  if (socket_desc == -1) {
    printf("Could not create socket");
  }

  server.sin_addr.s_addr = inet_addr(proxy_ip);
  server.sin_family = AF_INET;
  server.sin_port = htons(atoi(proxy_port));

  if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0) {
    puts("connect error");
    return 1;
  }

  puts("Connected");
  
  receive_file(socket_desc, file_name, web_url, "", username, password);
  
  if(downloadLogo) {
    char* img_url = get_logo_url(file_name);
    //puts(img_url);
    receive_file(socket_desc, img_name, web_url, img_url, username, password);
  }
  
  close(socket_desc);
  

  return 0;
}