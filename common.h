
void kill (const char *msg) {
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

int sock_read (const io_t io, char *buffer, const int size)
{ // bm sock_read
time_t basetime;
time (&basetime);

int len = -1;
while (len < 0)
{
len = read (io.fd, buffer, size);
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
} // sock_read

int sock_writeold (const io_t io, const char *buffer, const int size)
{ // bm sock_writeold

time_t basetime;
time (&basetime);

int len = -1;
while (len < 0)
{
len = write (io.fd, buffer, size);
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
} // sock_write_old

int ssl_nbread (const io_t io, char *buffer, const int size)
{ // bm ssl_NBread
time_t basetime;
time (&basetime);

int len = -1;
while (len < 0)
{
len = SSL_read (io.ssl, buffer, size);
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
} // ssl_nbread

int ssl_writeold (const io_t io, const char *buffer, const int size)
{ // bm ssl_writeold
time_t basetime;
time (&basetime);

int len = -1;
while (len < 0)
{
len = SSL_write (io.ssl, buffer, size);
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
} // ssl_write_old


int sock_setnonblock (const int fd)
{
if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) == -1)
    return (-1);

    return 1;
} // sock_setnonblock
