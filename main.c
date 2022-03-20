#include "main.h"
#include "common.h"
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

int send_txt (session_t *session, const char *txt)
{ // bm send txt
int len = strlen (txt);

//printf ("sending: %s\n", txt);

int total_len = (len + strlen(hthead) + strlen(conttxt) + strlen(connclose) + strlen (contlen) +3);


session->out.p = (char *) malloc (total_len);
if (session->out.p == NULL) killme ("no malloc");

session->total = total_len;
session->out.len = total_len;
session->procint = 1;

sprintf (session->out.p, "%s%s%s%s%d\n\n%.*s", hthead, conttxt, connclose, contlen, len, len, txt);

printf ("session p from sendtxt\n%s\n", session->out.p);

return 1;


//io->writer (io->io, outbuff.p, outbuff.len);

//sock_writeold (fd, outbuff.p, outbuff.len);
return 1;
} // send_txt

void softclose (const int fd, buffer_t *inbuff)
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

int process_request (session_t *session) {
// bm process request
buffer_t inbuff;
char in[string_sz];
inbuff.p = in;
inbuff.max = string_sz;

inbuff.len = session->reader (session->io, inbuff.p, inbuff.max);

printf ("request\n%.*s\n", inbuff.len, inbuff.p);


/*
request.method = inbuff.p[0];
char *p1 = (char *) memchr (inbuff.p, 32, inbuff.len);
int d1 = p1 - inbuff.p;

char *p2 = (char *) memchr (inbuff.p + d1 + 1, 32, inbuff.len);                          
int d2 = p2 - inbuff.p;
int len = d2 - d1;

memcpy (request.uri, inbuff.p + d1, len);
request.uri[len+1] = 0;

return request;
*/
return 1;
} // process new request

/*
void *thread_function (void * arg)
{ // bm thread F
const struct thread_data tdata = *(struct thread_data *) arg;
struct ipc_table *ipc = tdata.ipc;

//create poll
const int tpollmax = 2;
struct pollfd evtable [tpollmax], evback [tpollmax];
memset (evback, 0, sizeof (struct pollfd) * tpollmax);

int client_wait = 0;

// add tdata.pipes[0] to poll
evback[0].fd = tdata.pipes[0];
evback[0].events = POLLIN;

//thread loop
while (1) {
buffer inbuff;
char inb [dstring_sz];
inbuff.p = inb;
inbuff.max = dstring_sz;

//poll
memcpy (evtable, evback, sizeof(evback));

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

if (!strncmp (inbuff.p, "new", inbuff.len)) {
printf ("%d thread recv new conn\n", ipc->threadid);
evtable[1].fd = ipc->fd;
client_wait = 1;
evback[1].fd = ipc->fd;
evback[1].events = POLLIN;
continue;

} // if new connection
continue;
}

//  client loop, process until TO
//while (1) { // thread client loop
inbuff.len = ipc->reader (ipc->io, inbuff.p, inbuff.max);
//if (inbuff.len == -1) {
//ipc->busy = false;
//close (ipc->io.fd);
//break;
//}// if client timeout (READ LOOP)
printf ("inbuff: %s\n", inbuff.p);

struct request_data request = process_request (inbuff);
trim (request.uri);

//printf ("%s\n", inbuff.p);

//printf ("%s\n", request.uri);
//struct request_data request;
//strcpy (request.uri, "/index.html");

send_txt (ipc, request.uri);
printf ("%s\n", request.uri);
evback[1].fd = 0;
evback[1].events = 0;
ipc->busy = false;
//exit (0);
//} // handle client loop until TO
 

} // thread loop

return NULL;  
} // thread_function
*/


int main (int argc, char **argv)
{
// bm main top

SSL_CTX *ctx;
ctx = create_context();
configure_context(ctx);

sigset_t set; 

sigemptyset(&set);
sigaddset(&set, SIGPIPE);
if (pthread_sigmask(SIG_BLOCK, &set, NULL)  != 0)
 killme("pthread_sigmask");

if (argc == 2) {
if (!strcmp (argv[1], "-f")) {
load_settings (argv[2]); 
} // if -f
}else{ // if argc =2

load_settings ("config/server.txt");   
 // if config different
} // load config

total_users = 0;

/*
printf ("break\n\n");
printf ("use main: %d\n", settings.use_main);
printf ("thread count: %d\n", settings.thread_count);
printf ("HTTP mode: %d\n", settings.http_mode);
printf ("users per thread: %d\n", settings.users_per_thread);
printf ("editor: %s\n", settings.editor);
printf ("base_dir: %s\n", settings.base_dir);
printf ("internal: %s\n", settings.internal);
exit (0);
*/

// create poll
struct pollfd evtable[settings.users_per_thread], pollreg[settings.users_per_thread];
memset (pollreg, 0, sizeof (struct pollfd) * settings.users_per_thread);

//open server sockets
int httpfd, httpsfd;
httpfd = prepsocket (settings.http_port);
if (httpfd ==-1) killme ("listen error");
printf ("HTTP waiting on port: %d\n", settings.http_port);
// add httpfd to poll
pollreg[0].events = POLLIN;
pollreg[0].fd = httpfd;

httpsfd = prepsocket (settings.https_port);
if (httpsfd == -1) killme ("sock error");
printf ("HTTPS waiting on port: %d\n", settings.https_port);
// add httpsfd to poll
pollreg[1].events = POLLIN;
pollreg[1].fd = httpsfd;

/*
// start threads
pthread_t thread_pool [tcount];
struct thread_data tdata [tcount];
struct ipc_table *ipc = (ipc_t *) malloc (sizeof(ipc_t) * tcount);
if (ipc == NULL) killme ("malloc error");
memset (ipc, 0, sizeof(struct ipc_table) * tcount);

for (int i = 0; i < tcount; ++i)
{
if (pipe2 (tdata[i].pipes, O_NONBLOCK) != 0)
//if (pipe (pipes[i].pipes) != 0)
killme ("pipe error");

tdata[i].threadid = i;
ipc[i].threadid = i;
ipc[i].busy = false;
tdata[i].ipc = &ipc[i];

pthread_create(&thread_pool[i], NULL, thread_function, (void *) &tdata[i]);
} // for init threads
*/

// init sessions in stack, may move to heap #if linux later
session_t sessions [settings.users_per_thread];
memset (sessions, 0, sizeof(sessions));

while (1) {// bm server loop top
struct sockaddr_in address;
socklen_t addrlen = sizeof(address);

memcpy (evtable, pollreg, sizeof (pollreg));

int nevents = poll(evtable, settings.users_per_thread, -1);
if (nevents ==-1) killme ("poll wait err");
// poll does not change struct order


if (evtable[0].fd == httpfd && evtable[0].revents & POLLIN) {
// accept new http
//printf ("new http connection (ev#: %d)\n", i);
int connfd = accept(httpfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
if (connfd ==-1)  {printf ("new conn failed!!\n"); continue;}
sock_setnonblock (connfd);

char str[INET_ADDRSTRLEN];
inet_ntop(address.sin_family, &address.sin_addr, str, INET_ADDRSTRLEN);
printf("new http connection from %s:%d\n", str, ntohs(address.sin_port));

int sessionid = addconn (sessions, pollreg, connfd, http);
printf ("added sessionid: %d connfd: %d\n", sessionid, connfd);


} /*else if (evtable[1].fd == httpsfd && evtable[1].revents & POLLIN) {
// accept new https
//printf ("new https connection (ev#: %d)\n", i);
int connfd = accept(httpsfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
if (connfd ==-1)  {printf ("new conn failed!!\n"); continue;}
sock_setnonblock (connfd);

char str[INET_ADDRSTRLEN];
inet_ntop(address.sin_family, &address.sin_addr, str, INET_ADDRSTRLEN);
printf("new https connection from %s.%d\n", str, ntohs(address.sin_port));

SSL *ssl = SSL_new(ctx);
SSL_set_fd(ssl, connfd);

int rt = ssl_nbaccept (ssl);
if (rt <= 0) {close (connfd);}
} // if https
*/

// loop through other poll events 

for (int i = 2; i < settings.users_per_thread; ++i)
{
if (evtable[i].revents & POLLIN)
{
int sessionid = getsessionby_fd (sessions, evtable[i].fd);
printf ("POLLIN got session id:%d\n", sessionid);
process_request (&sessions[sessionid]);
send_txt (&sessions[sessionid], "it worked");
pollreg[sessionid].events = POLLOUT;

} // if POLLIN

if (evtable[i].revents & POLLOUT)
{
int id = getsessionby_fd (sessions, evtable[i].fd);
printf ("POLLOUT got session id:%d\n", id);

sessions[id].writer (sessions[id].io, sessions[id].out.p, sessions[id].out.len);
buffer_t b;
char x[200];
b.max = 200;
softclose (sessions[id].sockfd, &b);

exit (0);
// write POLLOUT    
    
} // if POLLOUT    
    
    
} // for other poll event loop
} // main server loop
} // main



int getsessionby_fd (const session_t *sessions, const int fd) 
{ // bm get session by id
 int i;
    for (i = 2; i < settings.users_per_thread; ++i)
    if (sessions[i].sockfd == fd)
    break;
    
    if (i == settings.users_per_thread && sessions[i].sockfd == 0)
    return -1;
    
    return i;
    
} // get session by id

int addconn (session_t *bank, struct pollfd *pollreg, const int fd, const enum _protocol p)
{ // bm add user
int i;
for (i = 2; i < settings.users_per_thread; ++i) 
if (bank[i].use == false) 
break;
      
if (i == settings.users_per_thread && bank[i].use == false)
return -1;

bank[i].use = true;
bank[i].protocol = p;
bank[i].sockfd = fd;
bank[i].io.fd = fd; 
time (&bank[i].stamp);

if (p == http)  {
bank[i].writer = sock_writer;
bank[i].reader = sock_reader;
}else if (p == https) {
bank[i].writer = tls_writer;
bank[i].reader = tls_reader;
} // if

pollreg[i].fd = fd;
pollreg[i].events = POLLIN;

++total_users;

return i;    
} // add user

/*
struct _session {
enum _status {idle, in, out} status;
enum _protocol {http, https, ws, wss} protocol;
int localfd;
int sockfd;
io_t io;
int (*writer)(const io_t iop, const char *buffer, const int len);
int (*reader)(const io_t iop, char *buffer, const int len);
int (*cb)(const struct _session sess);
page_t out;
int thread_id;
unsigned int permissions;
int user_id;
bool use;
int procint;
time_t stamp;
struct _session *next;
};
typedef struct _session session_t;
*/


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


void load_settings (const char *fname) {
// bm load_settings
printf ("loading config file: %s\n", fname);

int fd = open (fname, O_RDONLY);
if (fd == -1) killme ("bad config file");


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

//printf ("ls: %s / %s\n", name, value);


if (!strcmp ("threads", name)){ settings.thread_count = atoi (value);

    
}else if (!strcmp ("http_port", name)) { settings.http_port = atoi (value);
}else if (!strcmp ("https_port", name)) { settings.https_port = atoi (value);
}else if (!strcmp ("editor", name)) { strcpy (settings.editor, value);
}else if (!strcmp ("base_dir", name)) { strcpy (settings.base_dir, value);
}else if (!strcmp ("internal", name)) { strcpy (settings.internal, value);
}else if (!strcmp ("users_per_thread", name)) { settings.users_per_thread = atoi(value);
}else if (!strcmp ("http_mode", name)) { settings.http_mode = atoi(value);

}else if (!strcmp ("use_main", name)) { settings.use_main = atoi(value);

}

//port & URL


//printf ("name- %s\n", name);


//printf ("value- %s\n", value);

 
feed = rtn;
} // WHILE
 
} // load settings



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



