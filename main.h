
#define _GNU_SOURCE
// standard c library
#include <stdio.h>
// for file stat
#include <sys/stat.h>
// string functions
#include <string.h>
// for malloc
#include <stdlib.h>
// for files and file descriptors
#include <fcntl.h>
// for read / write
#include <unistd.h>

// for sockets and such
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>


// library I created with some useful functions and declarations
#include "shared.h"
// my comm library, to hande sockets and kqueue / epoll
#include "comm.h"
// for internally generated pages, and related functions
#include "page.h"

#define edit_mode "/edit"

struct kiss_request_data
{ // bm struct request_data
enum _mode {file, edit} mode;
char method;
char ext [20];
char uri[smbuff_sz];
char path[smbuff_sz];
char full_path[smbuff_sz];
};

// bm func declarations
void kiss_process_new (queue_t *);
void load_settings (const char *);
void process_out (queue_t *que);

const char *default_settings_path = "../config/server.txt";
struct main_settings {
int kiss_port;
int http_port;
int max_malloc;
int min_block;
char base_path [name_holder];
char altbase_path [name_holder];
}settings = {10000, 9999, 1000000, 10000, 
".", "."
};
