#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <netdb.h>
#include <pthread.h>
//#include <sys/epoll.h>
#include <poll.h>
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

struct page_data
{
char *p;
char head [dstring_sz];
int procint;
int len;
int max;
};
typedef struct page_data page;

struct buffer_data
{
char *p;
int procint;
int len;
int max;
};
typedef struct buffer_data buffer;

struct main_settings {
int http_port;
int https_port;
// 0=no http, 1=http redirect, 2=full http mode 
int httpmode;



char editor [name_holder];
char internal [name_holder];
char base_dir [name_holder];
}settings = {9999, 5555, 0,
"aceeditor.htm", ".", "."};

union _io {
SSL *ssl;
int fd;
};
typedef union _io io_t;


struct ipc_table {
bool busy;
bool enc;
int fd;
int threadid;

io_t io;
int (*writer)(const io_t iop, const char *buffer, const int len);
int (*reader)(const io_t iop, char *buffer, const int len);
};
typedef struct ipc_table ipc_t;


struct thread_data {
int pipes[2]; 
int threadid;
struct ipc_table *ipc;
};

struct request_data {
char uri [string_sz];
char path [string_sz];
char fullpath [string_sz];
int fd;
char method;
enum emode {err, favico, root, dir, file} mode;
};


int ssl_nbaccept (SSL *ssl);
int prepsocket (const int PORT);
int get_thread (const struct ipc_table *ipc, const int tc);
void load_settings (const char *path);


