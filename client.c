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

#include <stdbool.h>

#include <time.h>

// for sockets and such
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
//#include <sys/types.h>

// for poll
#include <poll.h>

// library I created with some useful functions
#include "shared.h"

void load_settings (const char *);

const char *default_settings_path = "../config/client.txt";
struct main_settings {
int kiss_port;
int http_port;
char host [name_holder];
char uri [name_holder];
}settings = {10000, 9999, 
"localhost", "/index.htm"    
};

int main (int argc, char **argv) 
{ // bm main top

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

p = strstr (argv[i], "-uri");
if (p != NULL)
{
char temp [name_holder];    
strcpy (temp, argv[i]);
if (!split_value (temp, '=', settings.uri)) killme ("unexpected cmd line arg");
}// if uri

p = strstr (argv[i], "-host");
if (p != NULL)
{
char temp [name_holder];    
strcpy (temp, argv[i]);
if (!split_value (temp, '=', settings.host)) killme ("unexpected cmd line arg");
}// if uri


} // for

}else{
load_settings (default_settings_path);
} // if args

const int buffer_size = 200;

char buffer[buffer_size];

int sockfd, portno, n;

struct sockaddr_in serv_addr;
struct hostent *server;

portno = settings.http_port;
sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        killme("ERROR opening socket");

server = gethostbyname(settings.host); // get hostbyname
if (server == NULL) killme ("error resolving host");

bzero((char *) &serv_addr, sizeof(serv_addr));
serv_addr.sin_family = AF_INET;

bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
serv_addr.sin_port = htons(portno);

if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
    killme("ERROR connecting");

sprintf(buffer, "GET %s HTTP/1.1 OK\n", settings.uri);

n = write(sockfd,buffer,strlen(buffer));
if (n < 0) 
     killme("ERROR writing to socket");

bzero(buffer,buffer_size);

if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK) == -1)
    killme ("error making nonblock");
/*
n = read (sockfd, buffer, buffer_size);
printf ("%.*s\n", n, buffer);
n = read (sockfd, buffer, buffer_size);
printf ("%.*s\n", n, buffer);
*/

struct pollfd evtable;
evtable.fd = sockfd;
evtable.events = POLLIN;

char full_path [name_holder];

sprintf (full_path, "./dn%s", settings.uri);
printf ("full path: [%s]\n", full_path);
int localfd = open (full_path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
if (localfd == -1) killme ("error opening full_path");

unsigned long total = 0;
unsigned long progress = 0;
//for (int i = 0; i < 8; ++i)
int loopint = 0;
while (1)
{
++loopint;
printf ("loopint: %d\n", loopint);
int nevents = poll (&evtable, 1, 3000);
if (nevents == 0) {printf ("server timeout\n"); break;}
n = read (sockfd, buffer, buffer_size);
if (n == 0) {printf ("EOF\n"); break;}
if (n == -1) {printf ("read -1\n"); break;}


if (!total) {
char *p = (char *) memmem (buffer, n, "ength:", 6);
printf ("len: %d\n%.*s\n", n, n, buffer);
if (p == NULL) killme ("error locating length");
int d1 = p - buffer;
char *end = (char *) memchr (buffer + d1, 10, n - d1);
if (end == NULL) killme ("error locating end");
int d2 = end - buffer;

int len = d2 - d1;

char str_sz [name_holder];
memcpy (str_sz, buffer + d1 + 7, len - 7);
str_sz [len] = 0;
total = atol (str_sz);
continue;

} // if total not set yet

write (localfd, buffer, n);

progress += n;
if (progress == total) {
printf ("total recieved\n");
break;
}

//printf ("#%d\n%.*s\n", i, n, buffer);

evtable.revents = 0;    
} // while
close (localfd);
} // main bottom

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
}else if (!strcmp(line, "kiss_port")) {settings.http_port = atoi (value); 
}else if (!strcmp(line, "host")) {strcpy (settings.host, value); 
}else if (!strcmp(line, "uri")) {strcpy (settings.uri, value); 

} // if

    
} // while


free (buffer);
} // end load settings