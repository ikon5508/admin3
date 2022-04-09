
#include "comm.h"
// for sockets and structs
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
// standard c library
#include <stdio.h>
// for malloc
#include <stdlib.h>
// for strings
#include <string.h>

#include <fcntl.h>

static void killme (const char *msg) {
    printf ("%s\n", msg);
    exit (0);
} // killme

void node_delete (queue_t *que, node_t *node)
{// bm node delete
if (node == que->top) {
que->top = que->top->next;    
que->top->prev = NULL;
    
    
}else if (node == que->bottom) {
que->bottom = que->bottom->prev;
que->bottom->next = NULL;
    
}else if (node == que->current) {
que->current = que->top;
}

node_t *prev = node->prev;
node_t *next = node->next;

prev->next = next;
next->prev = prev;

free (node);
} // node delete

void node_change (queue_t *q, node_t *node, const que_mode_t mode)
{ // bm node change

node->set_mode = mode;

#ifdef __USE_KQUEUE__
short filt = 0;
switch (mode) {
 case QUE_R: filt = EVFILT_READ; break;
 case QUE_W: filt = EVFILT_WRITE; break;
 case QUE_RW: filt = EVFILT_READ | EVFILT_WRITE; break;
}
EV_SET(&q->kq_list[q->changes], node->connfd, filt, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, 0);
++q->changes;
#endif

#ifdef __USE_EPOLL__
struct epoll_event epadd;
epadd.data.fd = node->connfd;
switch (mode) {
 case QUE_R: epadd.events = EPOLLIN | EPOLLET; break;
 case QUE_W: epadd.events = EPOLLOUT | EPOLLET; break;
 case QUE_RW: epadd.events = EPOLLIN | EPOLLOUT | EPOLLET; break;
}

if (epoll_ctl(q->que_fd, EPOLL_CTL_MOD, node->connfd, &epadd) == -1) 
{printf ("epoll_ctl error\n"); exit (0);}
#endif

} // node change

int sock_setnonblock (const int fd)
{
 return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK); 
} // sock_setnonblock

#ifdef __USE_KQUEUE__
int que_wait (queue_t *q)
{ // bm kqueue que_wait
q->current = q->top;
int nevents = kevent(q->que_fd, q->kq_list, q->changes, q->kq_list, kstruct_sz, NULL);
q->changes = 0;

if (nevents ==-1) killme ("kqueue/epoll error");

node_t *nxt = q->top;
while (nxt != NULL) {
for (int i = 0; i < nevents; ++i)
{
if (q->kq_list[i].ident == nxt->connfd){
if ((q->kq_list[i].filter == EVFILT_READ) && (nxt->set_mode == QUE_R)){
    nxt->ready = true; /*printf ("node EVREAD\n");*/
  //      if (q->kq_list[i].filter == EVFILT_WRITE) printf ("also write\n");
}else if ((q->kq_list[i].filter == EVFILT_WRITE) && (nxt->set_mode == QUE_W)) {
    nxt->ready = true; /*printf ("node EVWRITE\n");*/
 //   if (q->kq_list[i].filter == EVFILT_READ) printf ("also read\n");
}
}

    
} // for nevents loop
nxt = nxt->next;
} // while node loop
memset (q->kq_list, 0, kstruct_sz * sizeof(struct kevent));
return nevents;
} // kqueue que_wait
#endif 

#ifdef __USE_EPOLL__
int que_wait (queue_t *q)
{ // bm epoll que_wait
q->current = q->top;

int nevents = epoll_wait(q->que_fd, q->epoll_list, kstruct_sz, -1);

if (nevents ==-1) killme ("kqueue/epoll error");

node_t *nxt = q->top;

while (nxt != NULL) {
for (int i = 0; i < nevents; ++i)
{
if (q->epoll_list[i].data.fd == nxt->connfd){
if ((q->epoll_list[i].events == EPOLLIN) && (nxt->set_mode == QUE_R)){
    nxt->ready = true; /*printf ("node EPOLLIN\n");*/
}else if ((q->epoll_list[i].events == EPOLLOUT) && (nxt->set_mode == QUE_W)) {
    nxt->ready = true; /*printf ("node EPOLLOUT\n");*/
}
}
} // for nevents loop
nxt = nxt->next;
} // while node loop
memset (q->epoll_list, 0, kstruct_sz * sizeof(struct epoll_event));

return nevents;
} // epoll que_wait
#endif 



void que_setup (queue_t *q, const int nodes_max)
{ // bm que setup

memset (q, 0, sizeof (queue_t));
q->nodes_max = nodes_max;

#ifdef __USE_KQUEUE__
q->que_fd = kqueue ();
#endif

#ifdef __USE_EPOLL__
q->que_fd = epoll_create (1);
#endif

if (q->que_fd ==-1)
    {printf ("error creating kqueue/epoll"); exit(0);}
} // que setup



node_t *que_add (queue_t *q, const int fd, const protocol_t protocol, const que_mode_t mode)
{ // bm que add
if (q->node_count == q->nodes_max)
return NULL;
++q->node_count;

node_t *new_node = (node_t *) calloc (1, sizeof(node_t));
if (new_node == NULL) return NULL;
new_node->connfd = fd;
new_node->set_mode = mode;
new_node->protocol = protocol;

if (q->top == NULL)
q->top = new_node;

if (q->bottom == NULL)
{
q->bottom = new_node;
}else{
node_t *old_bottom = q->bottom;
old_bottom->next = new_node;
q->bottom = new_node;
} // bottom logic

#ifdef __USE_KQUEUE__
short filt;
switch (mode) {
 case QUE_R: filt = EVFILT_READ; break;
 case QUE_W: filt = EVFILT_WRITE; break;
 case QUE_RW: filt = EVFILT_READ | EVFILT_WRITE; break;
}
EV_SET(&q->kq_list[q->changes], fd, filt, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, 0);
++q->changes;
#endif

#ifdef __USE_EPOLL__
struct epoll_event epadd;
epadd.data.fd = fd;
switch (mode) {
 case QUE_R: epadd.events = EPOLLIN | EPOLLET; break;
 case QUE_W: epadd.events = EPOLLOUT | EPOLLET; break;
 case QUE_RW: epadd.events = EPOLLIN | EPOLLOUT | EPOLLET; break;
}

if (epoll_ctl(q->que_fd, EPOLL_CTL_ADD, fd, &epadd) == -1) 
{printf ("epoll_ctl error\n"); exit (0);}
#endif

return new_node;   
} // que add

int prepserver (const int PORT)
{
int result = 0;
int optval = 1;

int server_fd = socket(AF_INET, SOCK_STREAM, 0);

struct sockaddr_in address;
int addrlen = sizeof(address);

memset(&address, 0, sizeof (address));
address.sin_family = AF_INET;

address.sin_addr.s_addr = INADDR_ANY;

address.sin_port = htons( PORT );
//memset(address.sin_zero, 0, sizeof address.sin_zero);

result = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval , sizeof(int));
if (result == -1)
	perror("error, reuse addr");

//result = setsockopt(server_fd, SOL_SOCKET, TCP_CORK,&optval , sizeof(int));
//if (result == -1)
//	logging("error, TCP-CORK", 100, 0);


//result = setsockopt(server_fd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));
//if (result == -1)
//	logging("error, linger", 100, 0);

result = bind(server_fd, (struct sockaddr *)&address,(socklen_t) sizeof(address));
if (result == -1)
	perror("error, bind");

result = listen(server_fd, 10);
if (result == -1)
	perror("error, reuse listen");

return (server_fd);
}// end prep server

