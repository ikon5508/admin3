#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <fcntl.h>

#define string_sz 1024
#define maxbuffer 100000
#define nameholder 100

#define tlsRTO 3
#define tlsWTO 3

// bm headers

struct buffer_data {
char *p;
int procint;
int len;
int max;
}; // buffer_data
typedef struct buffer_data buffer;

struct iospec {
int fd;
SSL *ssl;
int sb;
char sbname [nameholder];
}; // iospec

const struct timespec msdelay = {0, 1000000};
struct timespec tsdump;

const char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello World!";

int create_socket(int port);
SSL_CTX *create_context();
int sock_setnonblock (const int fd);
void configure_context(SSL_CTX *ctx);
int tls_nbaccept (SSL *ssl);
int tls_reciever (struct iospec io, char *in, const int max);
int tls_sender (struct iospec io, char *in, const int max, int complete);

int main(int argc, char **argv)
{
int sock;
SSL_CTX *ctx;

ctx = create_context();

configure_context(ctx);

sock = create_socket(55555);

// main server loop
while(1) {
struct sockaddr_in addr;
unsigned int len = sizeof(addr);
SSL *ssl;

int clientfd = accept(sock, (struct sockaddr*)&addr, &len);
 if (clientfd < 0) {
perror("Unable to accept");
exit(EXIT_FAILURE);
}
  
   
sock_setnonblock (clientfd);
//printf ("rt: %d\n", rt);


ssl = SSL_new(ctx);
 SSL_set_fd(ssl, clientfd);

if (tls_nbaccept (ssl) ==-1)
    continue;

struct iospec io;
io.fd = clientfd;
io.ssl = ssl;

buffer inbuff;
inbuff.max = string_sz;
char inb [string_sz];
inbuff.p = inb;

inbuff.len = tls_reciever (io, inbuff.p, inbuff.max);

  printf ("%s\n", inbuff.p);
 
 tls_sender (io, hello, strlen (hello), 1);
 
SSL_shutdown(ssl);
SSL_free(ssl);


close(clientfd);
} // server loop

close(sock);
SSL_CTX_free(ctx);
} // main
// bm end_main

int tls_sender (struct iospec io, char *in, const int max, int complete)
{
time_t basetime;
time (&basetime);
 
int len =-1;
while (len < 0)
{
len = SSL_write (io.ssl, in, max);
if (len == -1 || len == 0)
{
nanosleep (&msdelay, &tsdump);
time_t deadtime;
time (&deadtime);
deadtime -= basetime;

if (deadtime >= tlsWTO)
	{ return -1;}

continue;    
} // if -1

} // while

return len;
    
} // tls_sender

int tls_reciever (struct iospec io, char *in, const int max)
{
time_t basetime;
time (&basetime);
 
int len =-1;
while (len < 0)
{
len = SSL_read (io.ssl, in, max);
if (len == -1 || len == 0)
{
nanosleep (&msdelay, &tsdump);
time_t deadtime;
time (&deadtime);
deadtime -= basetime;

if (deadtime >= tlsRTO)
	{ return -1;}

continue;    
} // if -1

} // while

return len;
    
} // tls_reciever

int tls_nbaccept (SSL *ssl)
{
int rt = -1;
while (rt <= 0)
{
rt = SSL_accept(ssl);
//printf ("rt: %d", rt);

if (rt > 0)
return 1;

if (rt <= 0)
{
nanosleep (&msdelay, &tsdump);
int rt2 = SSL_get_error(ssl, rt);

if (rt2 == SSL_ERROR_WANT_WRITE || rt2 == SSL_ERROR_WANT_READ) {
//printf ("want read / write\n");
continue;

} else if (rt2 == SSL_ERROR_WANT_CONNECT || rt2 == SSL_ERROR_WANT_ACCEPT){
//printf ("want connect / accept\n");
continue;
    
} else {
//printf ("non recoverable error\n");
    return -1;
} //if rt2

} // if rt-1

} // while
return -1;
} // tls_nbaccept

int create_socket(int port)
{
int s;
int optval = 1;
struct sockaddr_in addr;

addr.sin_family = AF_INET;
addr.sin_port = htons(port);
addr.sin_addr.s_addr = htonl(INADDR_ANY);

s = socket(AF_INET, SOCK_STREAM, 0);
if (s < 0) {
perror("Unable to create socket");
exit(EXIT_FAILURE);
}

if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval , sizeof(int)) == -1){
perror("error, reuse addr");
exit (EXIT_FAILURE);
}

if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
perror("Unable to bind");
exit(EXIT_FAILURE);
}

if (listen(s, 10) < 0) {
perror("Unable to listen");
exit(EXIT_FAILURE);
}

return s;
}

SSL_CTX *create_context()
{
const SSL_METHOD *method;
SSL_CTX *ctx;

method = TLS_server_method();

ctx = SSL_CTX_new(method);
if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
}

    return ctx;
} // SSL_CTX_NEW

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
} // configure context

int sock_setnonblock (const int fd)
{
if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) == -1)
{
    perror("calling fcntl");

    return 0;
} // if
    return 1;
} // sock_setnonblock
