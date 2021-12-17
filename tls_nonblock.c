#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <fcntl.h>
int create_socket(int port)
{
    int s;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }

    if (listen(s, 1) < 0) {
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

int sock_setnonblock (const int fd)
{
if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) == -1)
{
    perror("calling fcntl");

    return 0;
} // if
    return 1;
} // sock_setnonblock
int main(int argc, char **argv)
{
    int sock;
    SSL_CTX *ctx;

    ctx = create_context();

    configure_context(ctx);

    sock = create_socket(55555);

    /* Handle connections */
    while(1) {
        struct sockaddr_in addr;
        unsigned int len = sizeof(addr);
        SSL *ssl;
        const char reply[] = "test\n";

        int client = accept(sock, (struct sockaddr*)&addr, &len);
   
   
int rt = sock_setnonblock (client);
//printf ("rt: %d\n", rt);
	if (client < 0) {
            perror("Unable to accept");
            exit(EXIT_FAILURE);
        }

        ssl = SSL_new(ctx);
       rt = SSL_set_fd(ssl, client);

//printf ("sst rt: %d\n", rt);
const char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello World!";
 unsigned char buffer [1000];

int proceed = 0;
rt = -1;
while (rt <= 0)
{
rt = SSL_accept(ssl);
//printf ("rt: %d", rt);

// sslget error here,returning -1 now

if (rt > 0)
    proceed = 1;

if (rt <= 0)
{
int rt2 = SSL_get_error(ssl, rt);

if (rt2 == SSL_ERROR_WANT_WRITE || rt2 == SSL_ERROR_WANT_READ) {
printf ("want read / write\n");
continue;

} else if (rt2 == SSL_ERROR_WANT_CONNECT || rt2 == SSL_ERROR_WANT_ACCEPT){
printf ("want connect / accept\n");
continue;
    
} else {
printf ("non recoverable error\n");
    break;
} //if rt2

} // if rt-1

} // while

if (proceed) {
  
		
		
//SSL_read (ssl, buffer, 1000);
  //printf ("%s\n", buffer);
 
time_t basetime;
time (&basetime);
int len =-1;
while (len < 0)
{
printf ("pre write\n");
len = SSL_write (ssl, hello, strlen(hello));
printf ("len: %d\n", len);
if (len == -1 || len == 0)
{
usleep (100000);
time_t deadtime;
time (&deadtime);
deadtime -= basetime;

if (deadtime >= 20)
	{ printf ("timeout w"); break;}
} // if -1

//if (len < out.len

} // while

SSL_shutdown(ssl);
SSL_free(ssl);


// SSL_write(ssl, hello, strlen(hello));
} // if proceed

close(client);
} // server loop

close(sock);
SSL_CTX_free(ctx);
}
