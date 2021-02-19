#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>

// www.axmag.com/download/pdfurl-guide.pdf
int main(int argc , char *argv[])
{
    int socket_desc;

    char *message;
    char server_reply[10000];
    char few_bytes[10000];
    char *filename = "file.jpeg";
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

    server.sin_addr.s_addr = inet_addr("23.195.74.33");
    server.sin_family = AF_INET;
    server.sin_port = htons( 80 );

    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("connect error");
        return 1;
    }

    puts("Connected\n");

    //Send request
    message = "GET /v1/images/shopdisney-logo-desktop_1f595224.jpeg?region=0,0,1536,300 HTTP/1.0\r\nHost: lumiere-a.akamaihd.net\r\n\r\n";

    if ( send(socket_desc , message , strlen(message) , 0) < 0)
    {
        puts("Send failed");
        return 1;
    }

    puts("Data Send\n");

    remove(filename);
    file = fopen(filename, "w");

    if (file == NULL) {
        printf("File could not opened");
    }

    int flag = 0;

    while (1)
    {
        int received_len = recv(socket_desc, server_reply , sizeof server_reply , 0);

        total_len += received_len;

        if (flag == 0) {

            int len = strlen(server_reply);
            int cnt = 0;
            int in = 0;

            for(int i = 0; i < len; i++) {
                if(in == 0 && server_reply[i] != '\r')
                    continue;
                else if(in == 0 && server_reply[i] == '\r')
                {
                    if(server_reply[i+1] == '\n' && server_reply[i+2] == '\r' && server_reply[i+3] == '\n')
                        in = 1, i+=3;
                }
                else if(in == 1)
                    few_bytes[cnt++] = server_reply[i];
            }

            few_bytes[cnt] = '\0';
            server_reply[len-cnt] = '\0';
            fwrite(few_bytes, cnt, 1, file);
            flag = 1;
            printf("%s", server_reply);
            printf("******************** Header separated ***********************");
        }
        else if(received_len) {
            fwrite(server_reply, received_len, 1, file);
        }

        printf("\nReceived byte size = %d\nTotal length = %d\n", received_len, total_len);
        memset(server_reply, '\0', sizeof server_reply);
        if (received_len == 0) {
            break;
        }
    }

    puts("Reply received\n");

    fclose(file);

    return 0;
}