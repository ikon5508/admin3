#include "main.h"

int main (int argc, char **argv) 
{ // bm main top
struct sockaddr_in address;
socklen_t addrlen = sizeof(address);


if (argc > 1) {
char *p;
p = strstr (argv[1], "-f");
if (p != NULL)
{
char temp [name_holder];
char temp2 [name_holder];
strcpy (temp, argv[1]);
if (!split_value (temp, '=', temp2)) killme ("unexpected cmd line arg");
load_settings (temp2);
}else{
load_settings (default_settings_path);
}// if alt settings



for (int i = 1; i < argc; ++i) {
p = strstr (argv[i], "-max_malloc");
if (p != NULL)
{
char temp [name_holder];
char temp2 [name_holder];
strcpy (temp, argv[i]);
if (!split_value (temp, '=', temp2)) killme ("unexpected cmd line arg");
settings.max_malloc = atoi (temp2);
continue;
}// if max_malloc

p = strstr (argv[i], "-min_block");
if (p != NULL)
{
char temp [name_holder];
char temp2 [name_holder];
strcpy (temp, argv[i]);
if (!split_value (temp, '=', temp2)) killme ("unexpected cmd line arg");
settings.min_block = atoi (temp2);
continue;
}// if max_malloc


} // for
}else{
load_settings (default_settings_path);    
} // if args

queue_t que;
que_setup (&que, 100); // to load kqueue / epoll second argument is max connections

int kiss_fd = prepserver (settings.kiss_port);
if (kiss_fd ==-1) killme ("problem loading kiss");
printf ("kiss fd:port, %d:%d\n", kiss_fd, settings.kiss_port);
que_add (&que, kiss_fd, server, QUE_R);

int http_fd = prepserver (settings.http_port);
if (http_fd ==-1) killme ("problem loading http");
printf ("http fd:port, %d:%d\n", http_fd, settings.http_port);
que_add (&que, http_fd, server, QUE_R);
int sloopcount = 1;
while (1)
{ // bm server loop top
printf ("server loop iteration: %d (waiting)\n", sloopcount);
++sloopcount;
//if (sloopcount == 5) killme ("exit 4, success");
int nevents = que_wait (&que);
//printf ("%d in\n", nevents);

//node_t *nxt = que.top;
while (que.current != NULL)
{
//que.current = nxt;
//printf ("node iteration ready:fd: %d:%d\n", nxt->ready, nxt->connfd);
if (que.current->ready == true)
{
if (que.current->connfd == kiss_fd || que.current->connfd == http_fd)
{
int connfd = accept(que.current->connfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
sock_setnonblock (connfd);

char str[INET_ADDRSTRLEN];
  inet_ntop(address.sin_family, &address.sin_addr, str, INET_ADDRSTRLEN);
printf("new connection from %d-%s:%d\n", connfd, str, ntohs(address.sin_port));

protocol_t prot = (connfd==http_fd)? http:kiss;
que_add (&que, connfd, prot, QUE_R);

que.current->ready = false;
continue;
} // if server fd

//printf ("connfd: %d ready\n", que.current->connfd);  


/*
#define ready_out 1
#define head_use 2
#define head_done 3
#define body_use 4
#define body_done 5
#define sndfile 6
#define sfcomplete 7
*/

if (que.current->state & ready_out) {
    process_out (&que);
    
}

if (!que.current->state) {
    if (que.current->protocol == kiss) 
    kiss_process_new (&que);
}// if / else regular connection handling

} // if ready

que.current = que.current->next;    
}// node loop

} // server loop
} // main bottom

void process_out (queue_t *que)
{ // bm process out

if (que->current->head != NULL) {
write (que->current->connfd, que->current->head, que->current->head_len);
free (que->current->head);
que->current->head = NULL;
que->current->head_len = 0;
} // if head


// if total > progress


int body_progress = que->current->body_progress;
long body_len = que->current->body_len;
int offset = body_len - body_progress;
if (offset < settings.min_block && que->current->total != que->current->progress) {
printf ("preparing offset\n");
memmove (que->current->body, que->current->body + body_progress, body_len - body_progress);

int readin = read (que->current->localfd, que->current->body, settings.max_malloc);

que->current->progress += readin;

body_len = readin + offset;
body_progress = 0;

que->current->body_len = body_len;
que->current->body_progress = 0;
} // if addjustment needed




int wrtn = write (que->current->connfd, que->current->body + body_progress, body_len - body_progress);

printf ("%d written\n", wrtn);

body_progress += wrtn;
if (body_progress == body_len && que->current->progress == que->current->total) {
printf ("writing complete, freeing and closing\n");
if (que->current->localfd > 1) close (que->current->localfd);
free (que->current->body);
//que->bottom = que->current->back;
//free (que->current);
node_change (que, que->current, QUE_R);
que->current->state = 0;
que->current->ready = 0;
}else{
printf ("multi-part writer\n");
que->current->body_progress = body_progress;
} // if / else done

} // process out



int parse_kiss_request (struct kiss_request_data *req, const buffer_t inbuff)
{ // bm parse_request
memset (req, 0, sizeof (struct kiss_request_data));

req->method = inbuff.p[0];

char *p1 = (char *) memchr (inbuff.p, 32, inbuff.len);
if (p1 == NULL) return 0; //killme ("process_request p1 error");
int d1 = p1 - inbuff.p + 1;

char *p2 = (char *) memchr (inbuff.p + d1, 32, inbuff.len - d1);
if (p2 == NULL) return 0;//killme ("process_request p2 error");
int d2 = p2 - inbuff.p;
memcpy (req->uri, inbuff.p + d1, d2 - d1);
int len = d2 - d1;
req->uri [d2] = 0;

p1 = strrchr (req->uri, (int) '.');
if (p1 == NULL) {return 0;}
d1 = p1 - req->uri;
memcpy (req->ext, req->uri + d1, len - d1);
len -= d1;
req->ext[len] = 0;

//if (que->current->protocol == kiss)
//{
    strcpy (req->path, req->uri);
    sprintf (req->full_path, "%s%s", settings.base_path, req->uri);
    
//}

//p1 = (char *) strstr (rtn.uri, edit_mode)


return 1;
} // parse kiss request

void kiss_process_new (queue_t *que)
{ // bm process new kiss
buffer_t inbuff;
char inb[smbuff_sz];
inbuff.p = inb;
inbuff.max = smbuff_sz;

inbuff.len = read (que->current->connfd, inbuff.p, inbuff.max);
if (inbuff.len == 0) {printf ("read null\n"); return;}
if (inbuff.len == -1) {printf ("read -1\n"); return;}


inbuff.p[inbuff.len] = 0;

struct kiss_request_data request;
if (!parse_kiss_request (&request, inbuff)) return;

//printf ("request: [%s]\nURI [%s]\n", inbuff.p, request.uri);
printf ("URI [%s]\n", request.uri);

if (request.method == 'G')
{
printf ("preparing kiss get_file\n");
struct stat finfo;
if (stat (request.full_path, &finfo) != 0)
{printf ("bad filename\n"); send_txt (que, "bad file name\n"); return;}

int localfd = open (request.full_path, O_RDONLY);
if (localfd == -1) {printf ("error opening\n"); send_txt (que, "error opening\n"); return;}

generate_head (que->current, request.ext, finfo.st_size);

int malloc_sz = (finfo.st_size > settings.max_malloc)? settings.max_malloc: finfo.st_size;

que->current->localfd = localfd;
que->current->total = finfo.st_size;

que->current->body_len = malloc_sz;
que->current->body = (char *) malloc (malloc_sz);
if (que->current->body == NULL) killme ("error malloc m:230");

int readin = read (localfd, que->current->body, malloc_sz);
if (readin != malloc_sz) killme ("read err");

que->current->progress = malloc_sz;
printf ("file prepared, malloc_sz %d total %ld\n", malloc_sz, finfo.st_size);

que->current->state = 1;
node_change (que, que->current, QUE_W);
return;
} // if http GET
// check_permissions (request);


// generate appropriate response
/*

const char *resp = "this is a response";
int resp_len = strlen (resp);

que->current->body = (char *) malloc (resp_len);
strcpy (que->current->body, resp);
que->current->state = 1;
que->current->total = resp_len;

node_change (que, que->current, QUE_W);
*/
} // process new

void load_settings (const char *path)
{ // bm load settings
struct stat finfo;

printf ("loading settings from: %s\n", path);

if (stat (path, &finfo) != 0) {printf ("bad settings file, using defaults\n"); return ;}

char *buffer = (char *) malloc (finfo.st_size +1);
if (buffer == NULL) killme ("error in malloc, settings");

int fd = open (path, O_RDONLY);
if (fd < 0) killme ("error opening settings file");

int readin = read (fd, buffer, finfo.st_size);
if (readin != finfo.st_size) killme ("error reading settings file");

buffer[readin + 1] = 0;

//printf ("%s\n", buffer);

char *feed = buffer;

while (feed != NULL) 
{
char line [name_holder];
    feed = parse_line (line, feed);
    //printf ("ln: %s\n", line);
    
char value[name_holder];

split_value (line, ':', value);
//printf ("ln: %s - value: %s\n", line, value);

trim (line);
trim (value);

if (!strcmp(line, "kiss_port")) { settings.kiss_port = atoi (value);
}else if (!strcmp(line, "http_port")) {settings.http_port = atoi (value); 
}else if (!strcmp(line, "base_path")) {strcpy (settings.base_path, value); 
}else if (!strcmp(line, "altbase_path")) {strcpy (settings.altbase_path, value); 
}else if (!strcmp(line, "max_malloc")) {settings.max_malloc = atoi (value); 
}else if (!strcmp(line, "min_block")) {settings.min_block = atoi (value); 

} // if

    
} // while


free (buffer);
} // end load settings