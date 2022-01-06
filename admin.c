#include "admin.h"

struct request_data process_request (const struct args_data args, const buffer in)
{
// bm process_request
struct request_data request;
memset (&request, 0, sizeof (request));

char *p1 = NULL, *p2 = NULL;
int d1 = 0, d2 = 0;

char *end;

request.method = in.p [0];

if (request.method == 'G')
d1 = 4;
else
d1 = 5;

end = strchr (in.p + d1, 32);
if (end == NULL)
    term ("error end 19");

d2 = end - in.p;

strncat (request.url, in.p + d1, d2 - d1);

printf ("d1: %d, d2: %d\nURL: %s", d1, d2, request.url);

return request;

} // process_request

// bm main_top
int main (int argc, char **argv)
{
struct epoll_event ev, events[MAX_EVENTS];
int http_sock, n, conn_sock, nfds, epollfd;
int clientfd, tls_sock, c_sock;

struct sockaddr_in addr;
socklen_t addrlen = sizeof(addr);

SSL_CTX *ctx;

struct args_data args;
strcpy (args.base_path, ".");
strcpy (args.editor, "aceeditor.htm");
args.http_port = 8080;
args.tls_port = 55555;
args.c_port = 12345;


http_sock = prepsocket (args.http_port);
tls_sock = prepsocket (args.tls_port);
c_sock = prepsocket (args.c_port);

ctx = create_context();
configure_context(ctx);

epollfd = epoll_create(1);
if (epollfd == -1) 
    term ("epoll_create1");

ev.events = EPOLLIN;
ev.data.fd = http_sock;
if (epoll_ctl(epollfd, EPOLL_CTL_ADD, http_sock, &ev) == -1) 
    term("epoll_ctl: http_sock");

ev.data.fd = tls_sock;
if (epoll_ctl(epollfd, EPOLL_CTL_ADD, tls_sock, &ev) == -1) 
    term("epoll_ctl: tls_sock");

ev.data.fd = http_sock;
if (epoll_ctl(epollfd, EPOLL_CTL_ADD, c_sock, &ev) == -1) 
    term("epoll_ctl: c_sock");

printf ("server sock setup: http: %d, tls: %d, custom: %d\n", http_sock, tls_sock, c_sock);

struct sessionsDB sessions [MAX_EVENTS];
memset (&sessions, 0, sizeof (sessions));

// bm server_loop
while (1) 
{
SSL *ssl;
nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
if (nfds == -1) 
    term ("epoll_Wait");

for (n = 0; n < nfds; ++n) 
{
if (events[n].data.fd == http_sock || events[n].data.fd == tls_sock || events[n].data.fd == c_sock)
{
clientfd = accept(events[n].data.fd, (struct sockaddr *) &addr, &addrlen);
if (clientfd == -1) 
    term ("accept error");

if (fcntl(clientfd, F_SETFL, fcntl(clientfd, F_GETFL, 0) | O_NONBLOCK) ==-1)
    term ("fcntl");

if (events[n].data.fd == tls_sock)
{
ssl = SSL_new(ctx);
SSL_set_fd(ssl, clientfd);

if (tls_nbaccept (ssl) ==-1)
    {close (clientfd); continue;}

} // if tls, do handshake

ev.events = EPOLLIN | EPOLLET;
ev.data.fd = clientfd;
if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev) == -1) 
    term ("epoll_ctl error2");
printf ("accepted fd: %d\n", clientfd);

// add clientfd to sessions database to keep track of protocol

int mode;
if (events[n].data.fd == tls_sock)
    mode = 1;

if (events[n].data.fd == http_sock)
    mode = 2;

if (events[n].data.fd == c_sock)
    mode = 3;

if (add_session (sessions, clientfd, ssl, mode) ==-1)
    term ("session max hit!!");


continue;                   
} 
printf ("connection fd: %d", events[n].data.fd);


int sessionid = get_session (events[n].data.fd, sessions);

buffer inbuff;
char bf [string_sz * 2];
inbuff.p = bf;
inbuff.max = string_sz * 2;

//printf ("beginning to read: io type: %d", io.mode);

reciever (&sessions[sessionid], inbuff.p, inbuff.max);

//printf ("%s\n", inbuff.p);

struct request_data request = process_request (args, inbuff);
request.io = &sessions [sessionid];

char out [string_sz];
sprintf (out, "The URL is: %s", request.url);


send_txt(&sessions[sessionid], out);

close (sessions[sessionid].fd);
sessions[sessionid].fd = 0;
 // bm main-sess-management

if (sessions[sessionid].mode == 1)
{
SSL_shutdown(sessions[sessionid].ssl);
SSL_free(sessions[sessionid].ssl);
} //close ssl
    
} // for event loop
} // server loop

    SSL_CTX_free(ctx);

} // main


int http_sender (const struct sessionsDB *session, const char *buffer, int size)
{
// bm http_sender
if (size == 0)
    size = strlen (buffer);

//if (backdoor == 3)
//    return size;

time_t basetime;
time (&basetime);


int len = -1;
//nlogging ("bytes queued: ", out.len);

while (len < 0)
{
len = write (session->fd, buffer, size);
if (len == -1)
{
nanosleep (&msdelay, &tsdump);
time_t deadtime;
time (&deadtime);
deadtime -= basetime;

if (deadtime >= httpWTO)
	{return -1;}
} // if -1

//if (len < out.len

} // while

//if (backdoorfd)
//{
//write (backdoorfd, "\n......write......\n", 19);
//write (backdoorfd, buffer, len);
//}

if (len < size)
{
    printf ("incomplete sender\n");
    exit (0);
} // if incomplete


return len;
} // http_sender



int http_reciever (const struct sessionsDB *session, char *in, const int max)
{
// bm http_reciever
time_t basetime;
time (&basetime);
 
int len =-1;
while (len < 0)
{
len = read (session->fd, in, max);
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

//if (io->bd)
    //backdoor (io, in, len);


return len;
    
} // http_reciever



int tls_sender (const struct sessionsDB *session, const char *out, int len)
{
// bm tls_sender
time_t basetime;
time (&basetime);
 
int plen =-1;
while (plen < 0)
{
plen = SSL_write (session->ssl, out, len);
if (plen == -1 || plen == 0)
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

if (plen == len)
return plen;

printf ("incomplete sender\n");
exit (0);
} // tls_sender

int tls_reciever (const struct sessionsDB *session, char *in, const int max)
{
// bm tls_reciever
time_t basetime;
time (&basetime);
 
int len =-1;
while (len < 0)
{
len = SSL_read (session->ssl, in, max);
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

//if (io->bd)
    //backdoor (io, in, len);


return len;
    
} // tls_reciever


void term (const char *msg)
{
// bm term
    printf ("%s\n", msg);
    exit (0);
    
} // term


int prepsocket (int PORT)
{
// bm prepsocket
int result = 0;
int optval = 1;
struct linger ling;
ling.l_onoff=1;
ling.l_linger=4;

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
	term("error, reuse addr");

//result = setsockopt(server_fd, SOL_SOCKET, TCP_CORK,&optval , sizeof(int));
//if (result == -1)
//	logging("error, TCP-CORK", 100, 0);


//result = setsockopt(server_fd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));
//if (result == -1)
//	logging("error, linger", 100, 0);

result = bind(server_fd, (struct sockaddr *)&address,(socklen_t) sizeof(address));
if (result == -1)
    term ("error, bind");

result = listen(server_fd, 10);
if (result == -1)
	term("error, reuse listen");

return (server_fd);
}// end prep socket


int send_txt (const struct sessionsDB *session, const char *txt)
{
// bm send_txt
int len = strlen (txt);

char outbuffer [maxbuffer];
struct buffer_data outbuff;
outbuff.p = outbuffer;
outbuff.len = 0;
outbuff.max = maxbuffer;

outbuff.len = sprintf (outbuff.p, "%s%s%s%s%d\n\n%.*s", hthead, conttxt, connclose, contlen, len, len, txt);

sender (session, outbuff.p, outbuff.len);
return 1;
} // send_txt

SSL_CTX *create_context()
{
// bm creat_context
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
// bm configure_context
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

int tls_nbaccept (SSL *ssl)
{
//bm tls_nbaccept
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

int add_session (struct sessionsDB *sessions, const int fd, SSL *ssl, const int mode)
{
// bm add_session
for (int i = 0; i < MAX_EVENTS; ++i)
{
    //printf ("number: %d\n", sessions[i].fd);
    //sessions[i].fd *= -1;

if (sessions[i].fd == 0)
{    
// bm add_session
sessions[i].fd = fd;
sessions[i].mode = mode;
sessions[i].ssl = ssl;
printf ("session added to item: %d, fd: %d\n", i, fd);
return 1;
    
    // add time stamp later
} // if   
} // for
   return -1; 
} // add_session

int get_session (const int fd, const struct sessionsDB *sessions)
{
// bm get_session
printf ("get session!\n");

for (int i = 0; i < MAX_EVENTS; ++i)
{
if (sessions[i].fd == fd)
{

if (sessions[i].mode == 1)
    {sender = tls_sender; reciever = tls_reciever;}

if (sessions[i].mode == 2)
    {sender = http_sender; reciever = http_reciever;}

if (sessions[i].mode == 3)
    {sender = http_sender; reciever = http_reciever;}



printf ("returned item: %dsession mode: %d\n", i, sessions[i].mode);
return i;
}// if match
    
    
} // for
return -1;
    
} // get session


