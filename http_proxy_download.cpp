#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define ll long long
#define SIZE 10241

char ref[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char *websiteURL, *proxyIP, *proxyPort, *userID, *userPassword, *webName, *imgName, *buffer, *leftData, *rawCredentials, *requestHeader, *imgURL, *encodedCredentials, *headerData, *query, *comp;

bool shoudlDownload = false, part2 = false;

void initAll()
{
    websiteURL = (char *)calloc(SIZE, sizeof(char));
    proxyIP = (char *)calloc(SIZE, sizeof(char));
    proxyPort = (char *)calloc(SIZE, sizeof(char));
    userID = (char *)calloc(SIZE, sizeof(char));
    userPassword = (char *)calloc(SIZE, sizeof(char));
    webName = (char *)calloc(SIZE, sizeof(char));
    imgName = (char *)calloc(SIZE, sizeof(char));
}

void allocAll(ll val, char **a)
{
    websiteURL = a[1];
    proxyIP = a[2];
    proxyPort = a[3];
    userID = a[4];
    userPassword = a[5];
    webName = a[6];
    if (val == 8)
    {
        shoudlDownload = true;
        imgName = a[7];
    }
}

void eliminateTrailingHash()
{
    ll length = strlen(websiteURL);
    length--;
    while (websiteURL[length] == '/')
    {
        websiteURL[length] = '\0';
        length -= 1;
    }
}

void combineAuth()
{
    rawCredentials = (char *)calloc(SIZE, sizeof(char));
    strcat(rawCredentials, userID);
    strcat(rawCredentials, ":");
    strcat(rawCredentials, userPassword);
}

char *imgPath()
{
    char *ans = (char *)calloc(SIZE, sizeof(char));

    if (!strcmp(query, "SRC="))
    {
        char *comp = (char *)calloc(SIZE, sizeof(char));
        FILE *fileptr;
        fileptr = fopen(webName, "r");

        if (fileptr == NULL)
        {
            printf("Error has occurred. File could not be opened\n");
            return ans;
        }

        while (fscanf(fileptr, "%s", comp) != EOF)
        {
            ll len = strlen(comp);

            if (len <= strlen(query + 2))
                continue;

            char inp[SIZE], otp[SIZE];
            ll idx = 0;
            strcpy(inp, comp);
            inp[4] = '\0';

            if (!strcmp(query, inp))
            {
                for (ll i = strlen(query) + 1; comp[i] != '"'; i++)
                {
                    otp[idx] = comp[i];
                    idx += 1;
                }

                otp[idx] = '\0';
                strcpy(ans, otp);
                fclose(fileptr);
                return ans;
            }
        }
        fclose(fileptr);
        return ans;
    }
    else
    {
        char *str = strstr(leftData, query);

        if (str == NULL or !strlen(str))
            return ans;

        str = strstr(str, "//");

        if (str == "NULL" or !strlen(str))
            return ans;

        ll idx = 0;

        for (int i = 2; str[i] != '"'; i++)
            ans[idx++] = str[i];

        return ans;
    }
}

bool redirectionCheck()
{
    ll idx = 0;
    char *temp = (char *)calloc(SIZE, sizeof(char));

    for (ll i = 0; i < strlen(headerData); i++)
    {
        if (headerData[i] == '\r')
        {
            idx = i;
            break;
        }

        temp[i] = headerData[i];
    }

    temp[idx] = '\0';
    char *token = strtok(temp, " ");
    token = strtok(NULL, " ");

    if (!strcmp(token, "200"))
        return false;

    if (token[0] != '3')
        return false;

    websiteURL = (char *)calloc(SIZE, sizeof(char));
    query = (char *)calloc(SIZE, sizeof(char));
    strcat(query, "HREF=");
    char *redirectURL = imgPath();

    if (!strlen(redirectURL))
        return false;

    printf("Redirect to : %s\n", redirectURL);
    strcpy(websiteURL, redirectURL);
    return true;
}

int getRequest(ll socket_id)
{
    requestHeader = (char *)calloc(SIZE, sizeof(char));
    strcat(requestHeader, "GET http://");
    strcat(requestHeader, websiteURL);
    strcat(requestHeader, "/");

    if (part2)
    {
        query = (char *)calloc(SIZE, sizeof(char));
        strcat(query, "SRC=");
        imgURL = imgPath();
        strcat(requestHeader, imgURL);
    }

    strcat(requestHeader, " HTTP/1.1\r\nHost: ");
    strcat(requestHeader, websiteURL);
    strcat(requestHeader, "\r\nProxy-Authorization: Basic ");
    strcat(requestHeader, encodedCredentials);
    strcat(requestHeader, "\r\nConnection: close\r\n\r\n");
    printf("%s\n", requestHeader);

    if (send(socket_id, requestHeader, strlen(requestHeader), 0) < 0)
    {
        puts("Send failed");
        return -1;
    }
}

char *AuthEnocoder(char s[])
{
    ll idx, bits = 0, pd = 0, cnt = 0, res = 0, m = 0;
    char *ans = (char *)calloc(SIZE, sizeof(char));
    ll length = strlen(s);

    for (ll i = 0; i < length; i += 3)
    {
        res = cnt = bits = 0;

        for (ll j = i; j < length && j <= i + 2; j++)
        {
            res <<= 8;
            res |= s[j];
            cnt += 1;
        }

        bits = cnt * 8;
        pd = bits % 3;
        ll t;

        while (bits)
        {
            (bits >= 6) ? (t = bits - 6, idx = (res >> t) & 63, bits -= 6)
                        : (t = 6 - bits, idx = (res << t) & 63, bits = 0);

            ans[m] = ref[idx];
            m += 1;
        }
    }

    for (ll i = 1; i <= pd; i++)
    {
        ans[m] = '=';
        m += 1;
    }

    ans[m] = '\0';
    return ans;
}

ll ConnectToSock()
{
    ll socket_id;
    struct sockaddr_in server;
    socket_id = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_id == -1)
    {
        printf("Could not create socket !");
        return -1;
    }

    server.sin_addr.s_addr = inet_addr(proxyIP);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(proxyPort));
    //setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &(ll){1}, sizeof(int));

    if (connect(socket_id, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("Connect Error !");
        return -1;
    }

    printf("Connected\n\n");
    return socket_id;
}

ll separateHeaders(ll readLen)
{
    ll f = 0, idx = 0;

    for (ll i = 0; i < readLen; i++)
    {
        if (f == 0)
        {
            if (buffer[i] != '\r')
            {
                continue;
            }
            else if (buffer[i] == '\r')
            {
                if (buffer[i + 1] == '\n' && buffer[i + 2] == '\r' && buffer[i + 3] == '\n')
                {
                    f = 1;
                    i += 3;
                    buffer[i] = '\0';
                }
            }
        }
        else
        {
            leftData[idx] = buffer[i];
            idx += 1;
        }
    }

    headerData = (char *)calloc(SIZE, sizeof(char));
    strcpy(headerData, buffer);
    leftData[idx] = '\0';
    return idx;
}

ll downloadContent(ll socket_id, char *fileName)
{
    ll recievedLen = 0, readLen = 0, f = 0, idx;
    buffer = (char *)calloc(SIZE, sizeof(char));
    leftData = (char *)calloc(SIZE, sizeof(char));

    eliminateTrailingHash();
    combineAuth();
    encodedCredentials = AuthEnocoder(rawCredentials);

    if (getRequest(socket_id) == -1)
        return -1;

    remove(fileName);
    FILE *fileptr = fopen(fileName, "w");

    if (fileptr == NULL)
    {
        printf("Error has occurred. File could not be opened\n");
        return -1;
    }

    do
    {
        readLen = read(socket_id, buffer, SIZE);
        recievedLen += readLen;

        if (!f)
        {
            f = 1;
            idx = separateHeaders(readLen);
            printf("%s\n", buffer);
            fwrite(leftData, 1, idx, fileptr);
        }
        else
        {
            fwrite(buffer, 1, readLen, fileptr);
        }
    } while (readLen > 0);

    fclose(fileptr);
    printf("Data recieved successfully !\n\n");

    if (!part2)
    {
        bool redirect = redirectionCheck();

        if (redirect)
        {
            printf("Redirecting, Please Wait.......\n\n");
            close(socket_id);
            ll new_socket_id = ConnectToSock();
            downloadContent(new_socket_id, fileName);
            return 0;
        }
    }

    return 0;
}

int32_t main(int argc, char **argv)
{
    initAll();
    allocAll(argc, argv);

    ll socket1 = ConnectToSock();

    if (socket1 == -1)
        return 0;

    ll res1 = downloadContent(socket1, webName);

    if (res1 == -1)
    {
        close(socket1);
        return 0;
    }

    close(socket1);

    if (shoudlDownload)
    {
        part2 = true;
        ll socket2 = ConnectToSock();

        if (socket2 == -1)
            return 0;

        ll res2 = downloadContent(socket2, imgName);

        if (res2 == -1)
        {
            close(socket2);
            return 0;
        }
        close(socket2);
    }
}