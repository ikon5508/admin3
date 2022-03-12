#include "main.h"
#include "common.h"

/*
void kill (const char *msg) {
printf ("%s\n", msg);
exit (0);
    
}
*/
int pipe_read (const int connfd, char *buffer, const int size)
{ // bm pipe_read
time_t basetime;
time (&basetime);

int len = -1;
while (len < 0)
{
len = read (connfd, buffer, size);
if (len == -1)
{
usleep (udelay);
time_t deadtime;
time (&deadtime);

deadtime -= basetime;
if (deadtime >= timeout)
	{return -1;}

} // if -1
} // while

return len;
} // pipe_read

int pipe_write (const int connfd, const char *buffer, int size)
{ // bm pipe_write
if (size == 0)
    size = strlen (buffer);
time_t basetime;
time (&basetime);

int len = -1;
while (len < 0)
{
len = write (connfd, buffer, size);
if (len == -1)
{
usleep (udelay);
time_t deadtime;
time (&deadtime);
deadtime -= basetime;

if (deadtime >= timeout)
	{return -1;}
} // if -1

} // while
return len;
} // pipe_write_old


SSL_CTX *create_context()
{// bm create context
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
{// bm configure context
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

int send_txt (const ipc_t *io, const char *txt)
{ // bm send txt
int len = strlen (txt);

char outbuffer [smbuff];
struct buffer_data outbuff;
outbuff.p = outbuffer;
outbuff.len = 0;
outbuff.max = smbuff;

outbuff.len = sprintf (outbuff.p, "%s%s%s%s%d\n\n%.*s", hthead, conttxt, connclose, contlen, len, len, txt);

io->writer (io->io, outbuff.p, outbuff.len);

//sock_writeold (fd, outbuff.p, outbuff.len);
return 1;
} // send_txt

void softclose (const int fd, buffer *inbuff)
{ // bm softclose
shutdown (fd,  SHUT_WR);

//wait for client to close connection
int a = -1;
while (a) 
{
a=read(fd, inbuff->p, inbuff->max);
if(a < 0)
{
usleep (udelay);
}
if(!a) 
{
printf ("Connection closed by client\n");
} // if socket non blocked
} // for wait for close bit.

close(fd);
} // softclose

struct request_data process_request (const buffer inbuff) {
// bm process request
struct request_data request; 
memset (&request, 0, sizeof (request)); 

request.method = inbuff.p[0];
char *p1 = (char *) memchr (inbuff.p, 32, inbuff.len);
int d1 = p1 - inbuff.p;

char *p2 = (char *) memchr (inbuff.p + d1 + 1, 32, inbuff.len);                          
int d2 = p2 - inbuff.p;
int len = d2 - d1;

memcpy (request.uri, inbuff.p + d1, len);
request.uri[len+1] = 0;

return request;
} // process new request

void *thread_function (void * arg)
{ // bm thread F
const struct thread_data tdata = *(struct thread_data *) arg;
struct ipc_table *ipc = tdata.ipc;

//create poll
const int tpollmax = 2;
struct pollfd evtable [tpollmax];
memset (evtable, 0, sizeof (struct pollfd) * 2);

int client_wait = 0;

// add tdata.pipes[0] to poll
evtable[0].fd = tdata.pipes[0];
evtable[0].events = POLLIN;

//thread loop
while (1) {
buffer inbuff;
char inb [dstring_sz];
inbuff.p = inb;
inbuff.max = dstring_sz;

//poll
int nevents = poll(evtable, tpollmax, -1);
if (nevents == 0) { 
// delete from poll / epoll
close (ipc->io.fd);
ipc->busy = false;
continue;
} // if client timed out (EPOLL)

//for (int i = 0;i < tpollmax; ++i) {
if (evtable[0].fd == tdata.pipes[0] && evtable[0].revents & POLLIN){
inbuff.len = pipe_read (tdata.pipes[0], inbuff.p, inbuff.max);

if (!strcmp (inbuff.p, "new")) {
printf ("thread recv new conn\n");
evtable[1].fd = ipc->fd;
client_wait = 1;
} // if new connection

}

//  client loop, process until TO
while (1) { // thread client loop
inbuff.len = ipc->reader (ipc->io, inbuff.p, inbuff.max);
if (inbuff.len == -1) {
ipc->busy = false;
close (ipc->io.fd);
break;
}// if client timeout (READ LOOP)
//printf ("%s\n", inbuff.p);

struct request_data request = process_request (inbuff);
trim (request.uri);

//int get_perm (inbuff, reguest);
/*
if () {
 
 
 serv resource
 
    
}else {

send login 
    
}


*/
//printf ("%s\n", inbuff.p);

//printf ("%s\n", request.uri);

send_txt (ipc, request.uri);
printf ("%s\n", request.uri);

exit (0);
} // handle client loop until TO
 

} // thread loop

return NULL;  
} // thread_function

int main (int argc, char **argv)
{
// bm main top
const int tcount = 5;
const int maxpoll = 20;

SSL_CTX *ctx;
ctx = create_context();
configure_context(ctx);

//load_settings ("config/config.txt");

// iterate argv too!

// create poll
struct pollfd evtable [maxpoll];
memset (evtable, 0, sizeof (struct pollfd) * maxpoll);

//open server sockets
int httpfd, httpsfd;
httpfd = prepsocket (settings.http_port);
if (httpfd ==-1) kill ("listen error");
printf ("waiting on port: %d\n", settings.http_port);
// add httpfd to poll
evtable[0].events = POLLIN;
evtable[0].fd = httpfd;

httpsfd = prepsocket (settings.https_port);
if (httpsfd == -1) kill ("sock error");
printf ("waiting on port: %d\n", settings.https_port);
// add httpsfd to poll
evtable[1].events = POLLIN;
evtable[1].fd = httpsfd;

// start threads
pthread_t thread_pool [tcount];
struct thread_data tdata [tcount];
struct ipc_table *ipc = (ipc_t *) malloc (sizeof(ipc_t) * tcount);
if (ipc == NULL) kill ("malloc error");
memset (ipc, 0, sizeof(struct ipc_table) * tcount);

for (int i = 0; i < tcount; ++i)
{
if (pipe2 (tdata[i].pipes, O_NONBLOCK) != 0)
//if (pipe (pipes[i].pipes) != 0)
kill ("pipe error");

tdata[i].threadid = i;
ipc[i].threadid = i;
ipc[i].busy = false;
tdata[i].ipc = &ipc[i];

pthread_create(&thread_pool[i], NULL, thread_function, (void *) &tdata[i]);
} // for init threads

while (1) {// bm server loop top
struct sockaddr_in address;
socklen_t addrlen = sizeof(address);

int nevents = poll(evtable, maxpoll, -1);
if (nevents ==-1) kill ("poll wait err");
// poll does not change struct order

for (int i = 0; i < maxpoll; ++i) {
if (evtable[i].fd == httpfd && evtable[i].revents & POLLIN) {
// accept new http
printf ("new http connection (ev#: %d)\n", i);
int connfd = accept(httpfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
if (connfd ==-1)  {printf ("new conn failed!!\n"); continue;}
sock_setnonblock (connfd);

char str[INET_ADDRSTRLEN];
inet_ntop(address.sin_family, &address.sin_addr, str, INET_ADDRSTRLEN);
printf("new http connection from %s:%d\n", str, ntohs(address.sin_port));

// get available thread, pass connfd and pipe update signal
int tid = get_thread (ipc, tcount);
if (tid ==-1)
kill ("all threads taken!");

//printf ("putting connfd: %d into ipc %d\n", connfd, tid);
ipc[tid].io.fd = connfd;
ipc[tid].reader = sock_read;
ipc[tid].writer = sock_writeold;
ipc[tid].fd = connfd;
ipc[tid].busy = true;
ipc[tid].enc = false;
pipe_write (tdata[tid].pipes[1], "new", 3);

} else if (evtable[i].fd == httpsfd && evtable[i].revents & POLLIN) {
// accept new https
printf ("new https connection (ev#: %d)\n", i);
int connfd = accept(httpsfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
if (connfd ==-1)  {printf ("new conn failed!!\n"); continue;}
sock_setnonblock (connfd);

char str[INET_ADDRSTRLEN];
inet_ntop(address.sin_family, &address.sin_addr, str, INET_ADDRSTRLEN);
printf("new https connection from %s:%d\n", str, ntohs(address.sin_port));

SSL *ssl = SSL_new(ctx);
SSL_set_fd(ssl, connfd);

int rt = ssl_nbaccept (ssl);
if (rt <= 0) {close (connfd); break;}


//  update to ssl
// get available thread, pass connfd and pipe update signal
int tid = get_thread (ipc, tcount);
if (tid ==-1)
kill ("all threads taken!");

//printf ("putting connfd: %d into ipc %d\n", connfd, tid);
ipc[tid].io.ssl = ssl;
ipc[tid].reader = ssl_nbread;
ipc[tid].writer = ssl_writeold;
ipc[tid].fd = connfd;
ipc[tid].busy = true;
ipc[tid].enc = true;
pipe_write (tdata[tid].pipes[1], "new", 3);




//} else if (checkpipes (events[i].data.fd) != 0) {
// process IPC dafa from thread  
    
//} else {
// other monitored socket

} // if

} // epoll event loop

} // main server loop

} // main

int ssl_nbaccept (SSL *ssl){
// bm ssl_NBaccept
int rt = -1;
while (rt <= 0)
{
rt = SSL_accept(ssl);

if (rt <= 0)
{
usleep (udelay);
int rt2 = SSL_get_error(ssl, rt);
if (rt2 == SSL_ERROR_WANT_WRITE || rt2 == SSL_ERROR_WANT_READ) {
//printf ("want read / write\n");
continue;
} else if (rt2 == SSL_ERROR_WANT_CONNECT || rt2 == SSL_ERROR_WANT_ACCEPT){
//printf ("want connect / accept\n");
continue;
} else {
//printf ("non recoverable error\n");
break;
} //if rt2

} // if rt-1

} // while

// must be one to be succesful
// -1 dump connection
return rt;  
}// ssl_nbaccept

int get_thread (const struct ipc_table *ipc, const int tc)
{// bm get thread
for (int i = 0; i < tc; ++i)
{
if (ipc[i].busy == false)   
{



return ipc[i].threadid;

    
}// if
}// for

  
return -1;
}

/*
struct main_settings {

int HTTP_port;
int HTTPS_port;

int httpmode; // set to 1, redirect
int fork_request;

char editor [name_holder];
char internal [name_holder];
char base_dir [name_holder];
}settings = {9999, 5555, 1, 0,
"aceeditor.htm", ".", "."};
*/

/*
void load_settings (const char *path)
{

int fd = open (path, O_RDONLY);
if (fd < 0)
kill ("settings file not found");

int fsize = lseek (fd, 0, SEEK_END);

lseek (fd, 0, SEEK_SET);


char *data = malloc (fsize + 1);
if (data == NULL) 
kill ("cannot malloc load-settings\n");

int rin = read (fd, data, fsize);
if (rin != fsize)
kill ("read error 40");

data[fsize + 1] = 0;
printf ("data\n%s\nparse\n", data);


char *feed = data;
//for (int i = 0; i != 3; i++)
while (1)
{
char name [name_holder];
char value [name_holder];

char *rtn = parse_value (name, ':', value, feed);
if (rtn == NULL) break;

feed = rtn;


char section = 0;

if (!strcmp (name, "[ main ]"))
{section = 1; continue;}


    // to be completed
    
    


} // loop iterant


free (data);
close (fd);
}// load settings
*/

int prepsocket (const int PORT)
{
// bm prepsocket
int result = 0;
int optval = 1;
//struct linger ling;
//ling.l_onoff=1;
//ling.l_linger=4;

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
	{perror("error, bind");	return -1;} 


result = listen(server_fd, 10);
if (result == -1)
	perror("error, reuse listen");

return (server_fd);
}// end prep socket



