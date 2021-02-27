/* f20180141@hyderabad.bits-pilani.ac.in SHetty Karthik Ravindra */

/*
void initAll()
//initialize and allocate memory to all global variables

void allocAll(ll val, char **a)
//inita;ize variables with command line arguments

void combineAuth()
//combine credentials as per standard format username:password

char *imgPath()
//find the location of the image and extract its url

bool redirectionCheck()
//return 0 if response other than 30x, else call downloadContent function again after extracting new location

int getRequest(ll socket_id)
//formulate the get request header

char *AuthEnocoder(char s[])
//encode credentials using base64 encoder function

ll ConnectToSock()
//return sock_id every time you call this for a new socket connection

void checker(char *buffer, ll *i, ll *f)
//check for termination of response headers in the recieved buffer

ll separateHeaders(ll readLen)
//called for the first response, separates the response headers and data

int32_main(int argc, char** argv)
driver code for the who process of downloading webpage (if the website is for logo, then logo as well)
*/

/* ... */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#define ll long long
#define SIZE 99999

char ref[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *websiteURL, *proxyIP, *proxyPort, *userID, *userPassword, *webName, *imgName, *buffer, *leftData, *rawCredentials, *requestHeader, *imgURL, *encodedCredentials, *headerData, *query, *comp, *redirectURL, *tempURL;

ll shoudlDownload = 0, part2 = 0;

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

void eliminateTrailingHash()
{
    for (ll length = strlen(tempURL) - 1; tempURL[length] == '/'; length--)
        tempURL[length] = '\0';
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
        tempURL = (char *)calloc(SIZE, sizeof(char));

        strcpy(tempURL, websiteURL);
        eliminateTrailingHash();

        if (!strcmp(tempURL, "info.in2p3.fr"))
            shoudlDownload = 1;

        imgName = a[7];
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
    char *comp = (char *)calloc(SIZE, sizeof(char));
    FILE *fileptr = fopen(webName, "r");

    if (fileptr == NULL)
    {
        printf("Error !\n");
        return ans;
    }

    for (; fscanf(fileptr, "%s", comp) != EOF;)
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
            ll i = strlen(query) + 1;

            while (comp[i] != '"')
            {
                otp[idx] = comp[i];
                idx += 1;
                i += 1;
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

ll redirectionCheck()
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

    //retunr 0, if the response code does not starts with 3
    if (token[0] != '3')
        return 0;

    redirectURL = (char *)calloc(SIZE, sizeof(char));
    websiteURL = (char *)calloc(SIZE, sizeof(char));

    temp = strstr(headerData, "//");
    idx = 0;

    for (ll i = 2; temp[i] != '\r'; i++)
    {
        redirectURL[idx] = temp[i];
        idx += 1;
    }

    printf("Redirect to : %s\n", redirectURL);
    //replace wehsiteURL with the new redirection url
    strcat(websiteURL, redirectURL);
    return 1;
}

ll getRequest(ll socket_id)
{
    requestHeader = (char *)calloc(SIZE, sizeof(char));
    strcat(requestHeader, "GET http://");
    strcat(requestHeader, websiteURL);

    //check whether the function call is for downloading webpage or downloading the logo
    if (part2)
    {
        strcat(requestHeader, "/");
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

    return 0;
}

char *AuthEnocoder(char s[])
{
    ll idx, bits = 0, pd = 0, cnt = 0, res = 0, m = 0, i = 0;
    char *ans = (char *)calloc(SIZE, sizeof(char));
    ll length = strlen(s);

    while (i < length)
    {
        res = cnt = bits = 0;
        ll j = i;

        while (j < length && j <= i + 2)
        {
            res <<= 8;
            res |= s[j];
            cnt += 1;
            j += 1;
        }

        bits = cnt * 8;
        pd = bits % 3;
        ll t;

        for (; bits != 0;)
        {
            (bits >= 6) ? (t = bits - 6, idx = (res >> t) & 63, bits -= 6)
                        : (t = 6 - bits, idx = (res << t) & 63, bits = 0);

            ans[m] = ref[idx];
            m += 1;
        }

        i += 3;
    }

    i = 1;

    while (i <= pd)
    {
        ans[m] = '=';
        m += 1;
        i += 1;
    }

    ans[m] = '\0';
    return ans;
}

ll ConnectToSock()
{
    ll s_id;
    struct sockaddr_in serv1;
    s_id = socket(AF_INET, SOCK_STREAM, 0);

    if (s_id == -1)
    {
        printf("Could not create socket !");
        return -1;
    }

    serv1.sin_addr.s_addr = inet_addr(proxyIP);
    serv1.sin_family = AF_INET;
    serv1.sin_port = htons(atoi(proxyPort));
    setsockopt(s_id, SOL_SOCKET, SO_REUSEADDR, &(ll){1}, sizeof(int));

    if (connect(s_id, (struct sockaddr *)&serv1, sizeof(serv1)) < 0)
    {
        puts("Connect Error !");
        return -1;
    }

    printf("Connected\n\n");
    return s_id;
}

void checker(char *buffer, ll *i, ll *f)
{
    if (buffer[*i + 1] == '\n' && buffer[*i + 2] == '\r' && buffer[*i + 3] == buffer[*i + 1])
    {
        *f = 1;
        *i += 3;
        buffer[*i] = '\0';
    }
}

ll separateHeaders(ll readLen)
{
    ll f = 0, idx = 0;

    for (ll i = 0; i < readLen; i++)
    {
        (f == 0) ? (buffer[i] == '\r') ? checker(buffer, &i, &f) : (void)f : (void)(leftData[idx] = buffer[i], idx += 1);
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

    for (readLen = 1; readLen > 0;)
    {
        readLen = read(socket_id, buffer, SIZE);
        recievedLen += readLen;

        (f == 0) ? (f = 1,
                    idx = separateHeaders(readLen),
                    printf("%s\n", buffer),
                    fwrite(leftData, 1, idx, fileptr))
                 : (fwrite(buffer, 1, readLen, fileptr));
    }

    fclose(fileptr);
    printf("Data recieved successfully !\n\n");

    //check for redirection
    ll redirect = redirectionCheck();

    if (redirect)
    {
        printf("Redirecting, Please Wait.......\n\n");
        close(socket_id);
        ll new_socket_id = ConnectToSock();
        downloadContent(new_socket_id, fileName);
    }

    return 0;
}

int32_t main(int argc, char **argv)
{
    //initialize and allocate arguments
    initAll();
    allocAll(argc, argv);

    //open first socket for webpage download
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

    //open socket2 for logo download if shouldDownload variable 
    if (shoudlDownload)
    {
        part2 = 1;
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