#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include <unistd.h>

// Takes string to be encoded as input 
// and its length and returns encoded string 
char* get_logo_url() {
    char search_string[10241] = "SRC=";
    char word[10241];

    FILE *fptr;
    fptr = fopen("file.html", "r");

    while(fscanf(fptr, "%s", word) != EOF) {
        //puts(word);
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

            //puts(url);
            fclose(fptr);
            char* str = (char*)malloc(sizeof(char)*strlen(url));
            strcpy(str, url);
            return str;
        }
    }

    fclose(fptr);
    return "";
}

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
	{ 
		res_str[k++] = '='; 
	} 

	res_str[k] = '\0'; 

	return res_str; 

} 

// www.axmag.com/download/pdfurl-guide.pdf
int main(int argc , char *argv[])
{
    int socket_desc;
    char server_reply[10000];
    char few_bytes[10000];
    //char *filename = "file.html";
    char *filename = "file.gif";
    //char *filename = "file.png";


    int total_len = 0;

    int len;

    FILE *file = NULL;
    struct sockaddr_in server;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    server.sin_addr.s_addr = inet_addr("182.75.45.22");
    server.sin_family = AF_INET;
    server.sin_port = htons( 13128 );

    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("connect error");
        return 1;
    }

    puts("Connected\n");
    char *web_url = "http://info.in2p3.fr";

    int endlen = strlen(web_url);
    endlen--;

    while(web_url[endlen] == '/') {
        web_url[endlen] = '\0';
        endlen--;
    }

    char host_url[10241];
    int brk = 0;
    for(int i = 6; i < strlen(web_url); i++) 
        host_url[brk++] = web_url[i];

    host_url[brk] = '\0';

    char *auth_str = base64Encoder("csf303:csf303");
    //char auth_str[10241] = "Y3NmMzAzOmNzZjMwMw==";
    //puts(auth_str);
    //Send request
    
    char message[10241] = "GET ";
    strcat(message, web_url);
    strcat(message, "/");
    char* img_url = get_logo_url();
    puts(img_url);
    strcat(message,  img_url);
    //strcat(message, "http://go.com");
    //strcat(message, "http://lumiere-a.akamaihd.net/v1/images/shopdisney-logo-desktop_1f595224.jpeg?region=0,0,1536,300");
    //strcat(message, "http://info.in2p3.fr");
    strcat(message, " HTTP/1.1\r\nHost: ");
    strcat(message, host_url);
    //strcat(message, "lumiere-a.akamaihd.net");
    //strcat(message, "go.com");
    strcat(message, "\r\nProxy-Authorization: Basic ");
    strcat(message, auth_str);
    strcat(message, "\r\nConnection: close\r\n\r\n");

    if(send(socket_desc, message, strlen(message) , 0) < 0) {
        puts("Send failed");
        return 1;
    }

    remove(filename);
    file = fopen(filename, "w");

    if (file == NULL) {
        printf("File could not opened");
    }

    int flag = 0;

    while (1)
    {
        int received_len = recv(socket_desc, server_reply, sizeof (server_reply), 0);

        if (flag == 0) {
            int cnt = 0;
            int in = 0;

            for(int i = 0; i < received_len; i++) {
                if(in == 0 && server_reply[i] != '\r')
                    continue;
                else if(in == 0 && server_reply[i] == '\r')
                {
                    if(server_reply[i+1] == '\n' && server_reply[i+2] == '\r' && server_reply[i+3] == '\n')
                        in = 1, i+=3, server_reply[i] = '\0';
                }
                else if(in == 1)
                    few_bytes[cnt++] = server_reply[i];
            }

            few_bytes[cnt] = '\0';
            server_reply[cnt] = '\0';
            puts(server_reply);
            fwrite(few_bytes, cnt, 1, file);
            flag = 1;
            total_len += received_len;

            //printf("*********************** Header separated ***************************\n");
            //printf("\nreceived = %d\ncurrent img total = %d\n", received_len, total_len);
            //printf("Data : \n");
            //puts(few_bytes);
        }
        else if(received_len) {
            total_len += received_len;
            //printf("\nreceived = %d\ncurrent img total = %d\n", received_len, total_len);
            //printf("Data : \n");
            //puts(server_reply);
            fwrite(server_reply, received_len, 1, file);
        }

        if (received_len == 0) {
            break;
        }

        memset(server_reply, '\0', sizeof server_reply);
    }

    puts("\n***************** Complete Data recieved *******************************");

    fclose(file);
    return 0;
}
