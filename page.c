#include "page.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

int generate_head (node_t *node, const char *ext, unsigned long size)
{ // bm generate_head
char temp [smbuff_sz];
buffer_t out;
out.p = temp;
out.max = smbuff_sz;
  
const char *mime = NULL;

if (!strcmp (ext, ".txt")) {mime = conttxt;
}else if (!strcmp (ext, ".jpg")) {mime = contjpg;
}else if (!strcmp (ext, ".htm")) {mime = conthtml;
}else if (!strcmp (ext, ".html")) {mime = conthtml;
  
} // if mime mapping   
  
printf ("MIME: %s\n", mime); 
   

out.len = sprintf (out.p, "%s%s%s%lu\n\n", hthead, mime, contlen, size);

node->head = (char *) malloc (out.len);
if (node->head == NULL) killme ("error malloc head");
node->head_len = out.len;

strcpy (node->head, out.p);

  
    
return 1;
} // generate head

void send_txt (queue_t *que, const char *txt)
{ // bm send_txt
int len = strlen (txt);

char outbuffer [mdbuff_sz];
buffer_t out;
out.p = outbuffer;
out.len = 0;
out.max = mdbuff_sz;

out.len = sprintf (out.p, "%s%s%s%s%d\n\n%.*s", hthead, conttxt, connclose, contlen, len, len, txt);

//sock_buffwrite (fd, &outbuff);

que->current->body = (char *) malloc (out.len);
if (que->current->body == NULL) killme ("error malloc");
que->current->body_len = out.len;

strcpy (que->current->body, out.p);
que->current->state = 1;
node_change (que, que->current, QUE_W);
} // send_txt
