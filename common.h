int tls_reader (const io_t io, char *buffer, const int len)
{ // bm ssl_reader
    return SSL_read (io.ssl, buffer, len);
}// ssl_reader

int tls_writer (const io_t io, const char *buffer, const int len)
{// bm ssl_writer
    
    return SSL_write (io.ssl, buffer, len);
    
}// ssl_writer

int sock_reader (const io_t io, char *buffer, const int len)
{ // bm http_reader

return read (io.fd, buffer, len);

}// http_reader

int sock_writer (const io_t io, const char *buffer, const int len)
{ // bm http_writer

printf ("sockwriter called fd: [%d]\n%.*s\n", io.fd, len, buffer);

int rtn = write (io.fd, buffer, len);
    
printf ("return from write: %d\n", rtn);
return rtn;


}// http_writer


void killme (const char *msg) {
printf ("%s\n", msg);
exit (0);
}


char *parse_line (char *dest, const char *src)
{ // bm parse_line 
   char *rtn = strchr (src, 10);
    if (rtn == NULL) return NULL;
   
   
   int len = rtn - src;
    
    memcpy (dest, src, len);
    
    dest [len] = 0;
    
    return rtn +1;
 
} // parse_line

void split_value (const char d, char *value, char *src)
{ // bm split_value
char *p1 = strchr (src, (int) d);
if (p1 == NULL) return;

int len = strlen (src);


int d1 = p1 - src;
//memset (value, 0, valsz);
memcpy (value, src + d1 +1, len - d1 -1);
value [len - d1 -1] = 0;
src[d1] = 0;




} // parse value

void trim (char *totrim)
{ // bm void trim
int len = strlen (totrim);

//trim beginning
int count = 0;
for (int i = 0; i !=len; ++i)
{
int trigger = 0;
switch (totrim[i])
{
case 10: 
++count;
break;
case 13:
++count;
break;
case 32:
++count;
break;
  
default:
goto fin1;
  
} // switch
} // for
    
fin1:;
if (count)
{
memmove (totrim, totrim + count, len - count);
totrim [len - count] = 0;  // works partially
//memset (totrim + len - count, 0, count);
len -= count;
///printf ("trimming %d\n", count);
}

// trim end
for (int i = len -1; i > 0; --i)
{
// printf ("i: %d (%c - <%d>)\n", i, totrim[i], totrim[i]);
switch (totrim[i])
{
case 10: 
totrim[i] = 0;
break;
case 13:
totrim[i] = 0;
break;
case 32:
totrim[i] = 0;
break;

default:
i = 0;
} // switch
} // for
} // trim

int sock_setnonblock (const int fd)
{
if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) == -1)
    return (-1);

    return 1;
} // sock_setnonblock
