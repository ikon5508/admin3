#ifndef comm_h
#define comm_h 1

// comment / uncomment next 3 lines below to toggle poll / kqueue / epoll for *BSD or linux
//#define __USE_POSIXPOLL__ 1
#define __USE_KQUEUE__ 1
//#define __USE_EPOLL__ 1

#ifdef __USE_POSIXPOLL__
#include <poll.h> // posix compliant poll sys call
#endif

#ifdef __USE_KQUEUE__ // BSD / Mac specific code 
#include <sys/event.h>
#endif

#ifdef __USE_EPOLL__ // linux sepecific code
#include <sys/epoll.h>
#endif

#include <stdbool.h>

// defines struct sizes for use with kqueue / epoll / poll
#define kstruct_sz 100

enum _que_mode {QUE_R, QUE_W, QUE_RW};
typedef enum _que_mode que_mode_t;

enum _protocol {server, kiss, kissTLS, http, https};
typedef enum _protocol protocol_t;

/*states
0 ready to process new
1 ready out
2 head use
3 head done
4 body use
5 body done
*/

#define ready_out 1
#define head_use 2
#define head_done 3
#define body_use 4
#define body_done 5
#define sndfile 6
#define sfcomplete 7


struct _node {
    
#ifdef __USE_POSIXPOLL__
int poll_id;
#endif

int connfd;
int localfd;
protocol_t protocol;
enum _que_mode set_mode;
enum _que_mode get_mode;

bool ready;
unsigned int state;

char *head;
int head_len;
char *body;
int body_progress;
int body_len;

unsigned long progress;
unsigned long total;


int (*filesend)(struct _node *io);
int (*sender)(struct _node *io);
int (*reciever)(struct _node *io);

struct _node *next;
struct _node *prev;
};
typedef struct _node node_t;


struct _queue {
int node_count, nodes_max;
int que_fd;
node_t *top;
node_t *bottom;
node_t *current;

#ifdef __USE_POSIXPOLL__
struct pollfd evtable [kstruct_sz];
#endif

#ifdef __USE_EPOLL__
struct epoll_event epoll_list [kstruct_sz];
#endif

#ifdef __USE_KQUEUE__
struct kevent kq_list [kstruct_sz];
int changes;
#endif
};
typedef struct _queue queue_t;

void node_change (queue_t *q, node_t *node, const que_mode_t mode);
int sock_setnonblock (const int fd);
int prepserver (const int);
void que_setup (queue_t *, const int);
node_t *que_add (queue_t *, const int fd, const protocol_t protocol, const que_mode_t);
int que_wait (queue_t *);

#endif