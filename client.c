

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define name_holder 256
#define string_sz 1024
#define dstring_sz 2048

#define smbuff 10000
#define lgbuff 100000

#define timeout 3
#define mstimeout 3000

#define udelay 100000


union _io {
    SSL *ssl;
    int fd;
}io;
typedef union _io io_t;

int (*writer)(io_t io, const char *buffer, const int len);
int (*reader)(io_t io, char *buffer, const int len);

#include "common.h"


struct _settings {
int portno;
bool enc;
char host [string_sz];
char url [string_sz];
}settings;






void error(char *msg)
{
    perror(msg);
    exit(0);
}

SSL_CTX *create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_client_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx)
{
    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

struct timespec timediff(struct timespec start, struct timespec end) 
{
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0)
{ 
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec; 
        
} else { 
temp.tv_sec = end.tv_sec-start.tv_sec;
temp.tv_nsec = end.tv_nsec-start.tv_nsec;
}
return temp; 
    
}

void load_settings (const char *fname) {
// bm load_settings

int fd = open (fname, O_RDONLY);
if (fd == -1) kill ("bad config file");
char filed [lgbuff];
char name [name_holder];
char value [name_holder];

read (fd, filed, lgbuff);


char *feed = filed;

while (1)
{

char *rtn = parse_line (name, feed);
if (rtn == NULL) break;


//printf ("name: %s\n", name);

split_value (':', value, name);

trim (name);
trim (value);

if (!strcmp ("encrypt", name)) {
  if (!strcmp ("true", value)) settings.enc = true;

    
}else if (!strcmp ("host", name)) { strcpy (settings.host, value);
}else if (!strcmp ("port", name)) { settings.portno = atoi (value);
}else if (!strcmp ("URL", name)) { strcpy (settings.url, value);
}

//port & URL


//printf ("name- %s\n", name);


//printf ("value- %s\n", value);

 
feed = rtn;
} // WHILE
 
 
  
} // load settings

int main(int argc, char *argv[])
{
if (argc != 2) {
printf ("expected usage: %s /path/to/config/file", argv[0]);
exit (0);
}else{
load_settings (argv[1]);
} // if config

char buffer[smbuff];

int sockfd, n;

struct sockaddr_in serv_addr;
struct hostent *server;
SSL_CTX *ctx;

ctx = create_context();

configure_context(ctx);

SSL *ssl;

sockfd = socket(AF_INET, SOCK_STREAM, 0);
if (sockfd < 0) error("ERROR opening socket");

server = gethostbyname(settings.host); // get hostbyname
    if (server == NULL) error ("ERROR, no such host\n");

bzero((char *) &serv_addr, sizeof(serv_addr));
serv_addr.sin_family = AF_INET;
bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

serv_addr.sin_port = htons(settings.portno);

struct timespec start, end, diff;

clock_gettime (CLOCK_MONOTONIC, &start);

if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

if (settings.enc) {
ssl = SSL_new(ctx);
SSL_set_fd(ssl, sockfd);

writer = ssl_writeold;
reader = ssl_nbread;
io.ssl = ssl;
int rt = SSL_connect(ssl);
if (rt <= 0) {printf ("ssl connect error\n"); exit (0);}

}else {
  writer = sock_writeold;
  reader = sock_read;
  io.fd = sockfd; 
}
if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK) == -1)
  { printf ("calling fcntl\n"); exit (0);}



sprintf(buffer, "GET /index.htm HTTP/1.1 OK\n");

int rtn = writer (io, buffer, strlen(buffer));

bzero(buffer,smbuff);
printf ("%d writen\n", rtn);

rtn = reader (io, buffer, smbuff);


printf ("%s\n%d recieved", buffer, rtn);

clock_gettime (CLOCK_MONOTONIC, &end);

diff = timediff (start, end);
int millsec = diff.tv_nsec / 1000000;
printf ("took %ld:%d seconds \n", diff.tv_sec, millsec);


}
