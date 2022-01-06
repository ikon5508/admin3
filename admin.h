#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/ioctl.h>
//#define PORT 9999;

#define _XOPEN_SOURCE 700
#include <sys/types.h>
#include <dirent.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#ifdef __linux__
#include <linux/sockios.h>
#endif
#include <fcntl.h>
#include <signal.h>
#include <sys/epoll.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define string_sz 1024
#define maxbuffer 100000
#define buffermax 100000
#define nameholder 100

#define tlsRTO 3
#define tlsWTO 3
#define httpRTO 3
#define httpWTO 3

#define MAX_EVENTS 10

// bm headers
struct buffer_data {
char *p;int procint;
int len;
int max;
}; // buffer_data
typedef struct buffer_data buffer;


struct args_data {
char base_path [string_sz];
char editor [string_sz];

int c_port;
int tls_port;
int http_port;

int http_mode;
// 0: no http 1: http redirect, 2: full http
//enum exmode {tls, notls, sandbox} mode;

int sandbox;

}; // args_data

struct sessionsDB
{
int fd;
int mode;
SSL *ssl;
time_t tstamp;
// 1 - tls, 2 - http, 3 - custom
};

struct request_data
{
char url [string_sz];
char path [string_sz];
char fullpath [string_sz];

char method;
struct sessionsDB *io;

enum emode
{file, edit} mode;
};


const struct timespec msdelay = {0, 1000000};
struct timespec tsdump;
const char *hthead = "HTTP/1.1 200 OK\n";

const char *conticon = "Content-Type: image/x-icon\n";
const char *contjava = "Content-Type: text/javascript\n";
const char *conthtml = "Content-Type: text/html; charset=utf-8\n";
const char *conttxt = "Content-Type: text/plain\n";
const char *contjpg = "Content-Type: image/jpg\n";
const char *contpng = "Content-Type: image/png\n";
const char *contcss = "Content-Type: text/css\n";

const char *contmp4 = "Content-Type: video/mp4\n";

const char *connclose = "Connection: close\n";
const char *connka = "Connection: timeout=5, max=10\n";
const char *contlen = "Content-Length: ";

int http_sender (const struct sessionsDB *session, const char *buffer, int size);
int http_reciever (const struct sessionsDB *session, char *in, const int max);

int tls_reciever (const struct sessionsDB *session, char *in, const int max);
int tls_sender (const struct sessionsDB *session, const char *out, int len);


//int sb_sender (const struct iospec *io, const char *out, const int len);
//int sb_reciever (const struct iospec *io, char *in, const int max);

int (*reciever) (const struct sessionsDB *session, char *in, const int max);
int (*sender) (const struct sessionsDB *session, const char *out, const int len);

void term (const char *msg);
int send_txt (const struct sessionsDB *session, const char *txt);

int prepsocket (int PORT);

SSL_CTX *create_context();
void configure_context(SSL_CTX *ctx);

int tls_nbaccept (SSL *ssl);

int add_session (struct sessionsDB *sessions, const int fd, SSL *ssl, const int mode);

int get_session (const int fd, const struct sessionsDB *sessions);





//void backdoor (const struct iospec *io, const char *b, const int len);










