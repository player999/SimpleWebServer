
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFS 4096

// HTTP header for response
#define HEADERS "\
HTTP/1.0 200 OK\n\
Content-Type: text/html; charset=utf-8\n\
Cache-Control: no-cache\n\
Date: %s\n\
Server: Bydloserver\n\
Content-Length: %u\n\
Connection: Close\n\n\
"

//Root page
static char pageroot[] = 
"<html>"
"<head>"
"<title>Root Page</title>"
"</head>"
"<body>"
"<h1>This is Root page</h1>"
"<p><a href=\"/page1.html\">Page 1</p>"
"<p><a href=\"/page2.html\">Page 2</p>"
"</body>"
"</html>";

//Page 1
static char page1[] = 
"<html>"
"<head>"
"<title>Page 1</title>"
"</head>"
"<body>"
"<h1>This is Page 1</h1>"
"</body>"
"</html>";

//Page 2
static char page2[] = 
"<html>"
"<head>"
"<title>Page 2</title>"
"</head>"
"<body>"
"<h1>This is Page 2</h1>"
"</body>"
"</html>";

static char page404[] = 
"<html>"
"<head>"
"<title>404</title>"
"</head>"
"<body>"
"<h1>Whoops, page not found</h1>"
"</body>"
"</html>";

int process_packet(const char *inbuf, char *outbuf, int bufsize);
int make_response(const char *body, char *outbuf, int bufsize);

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, clilen, n, outlen, sendsize;
    char recv_buffer[BUFS], send_buffer[BUFS];
    struct sockaddr_in serv_addr, cli_addr;
    
    //Set up locale to default POSIX to correctly print date
    setlocale(LC_ALL, "C");

    //Create listening socket descriptor
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //Fill address structure for listening socket
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(8080);
    
    // Bind address to listening socket
    bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    
    //Make listening socket to listen
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    while(1){
        //Wait for connections
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        bzero(recv_buffer, BUFS);

        //Receive data from a new socket
        n = recv(newsockfd, recv_buffer, BUFS, 0);

        //Make response
        sendsize = process_packet(recv_buffer, send_buffer, BUFS);

        //Send response back
        n = send(newsockfd, send_buffer, sendsize, 0);

        //Close connection
        shutdown(newsockfd, SHUT_RDWR);

        //Close desciptor (every descriptor must be closed)
        close(newsockfd);
    }
    return 0; 
}

int process_packet(const char *inbuf, char *outbuf, int bufsize) 
{
    int urlen;
    char page[4096];
    int respsize;
    char *outdata;
    inbuf = &inbuf[4];
    for(urlen = 0; inbuf[urlen] != ' '; urlen++);
    bzero(page, 4096);
    memcpy(page, inbuf, urlen < 4096 ? urlen : 4095);

    //Route page
    if(!strcmp(page, "/")) outdata = pageroot;
    else if(!strcmp(page, "/page1.html")) outdata = page1;
    else if(!strcmp(page, "/page2.html")) outdata = page2;
    else outdata = page404;

    //Make response
    respsize = make_response(outdata, outbuf, bufsize);
}

int make_response(const char *body, char *outbuf, int bufsize)
{
    int offset;
    int nbytes;
    time_t curtime;
    char time_buffer[50];
    struct tm* curtime_info;

    bzero(outbuf, bufsize);
    bzero(time_buffer, sizeof(time_buffer));

    //Get time
    time(&curtime);

    //Convert time 
    curtime_info = gmtime(&curtime);

    //Make date for header
    strftime(time_buffer, 50, "%a, %d %b %y %T GMT", curtime_info);

    nbytes = strlen(body);

    //Fill header
    offset = snprintf(outbuf, bufsize, HEADERS, time_buffer, nbytes);

    //Append response body
    memcpy(outbuf + offset, body, nbytes);
    return nbytes + offset;
}
