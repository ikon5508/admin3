#include "shared.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
timer_c::timer_c ()
{ // bm timer constructor
clock_gettime (CLOCK_MONOTONIC, &start);
}// timer constructor

void timer_c::reset ()
{ // bm timer reset
clock_gettime (CLOCK_MONOTONIC, &start);
} // timer

void timer_c::stop () 
{ //bm timer end
clock_gettime (CLOCK_MONOTONIC, &end);
} //timer end

void timer_c::result (int *sec, int *ms)
{ // bm timer get diff
struct timespec temp;
//if ((end.tv_nsec-start.tv_nsec)<0)
if (end.tv_nsec < start.tv_nsec)
{ 
temp.tv_sec = end.tv_sec-start.tv_sec-1;
temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec; 
} else { 
temp.tv_sec = end.tv_sec-start.tv_sec;
temp.tv_nsec = end.tv_nsec-start.tv_nsec;
} // if

int millisec = temp.tv_nsec / 1000000;

*sec = temp.tv_sec;
*ms = millisec;

}// timer get diff
*/

void err_ctl (const int rslt, const char *msg) {
if (rslt < 0) {
printf ("%s\n", msg);
exit(0);
}
} //err_ctl


void killme (const char *msg) {
printf ("%s\n", msg);
exit(0);
} //killme


char *parse_line (char *dest, const char *src)
{ // bm parse_line 
   char *rtn = (char *) strchr (src, 10);
    if (rtn == NULL) return NULL;
   
   
   int len = rtn - src;
    
    memcpy (dest, src, len);
    
    dest [len] = 0;
    
    return rtn +1;
 
} // parse_line

int split_value (char *src, const char d, char *value)
{ // bm split_value
char *p1 = strchr (src, (int) d);
if (p1 == NULL) return 0;

int len = strlen (src);


int d1 = p1 - src;
//memset (value, 0, valsz);
memcpy (value, src + d1 +1, len - d1 -1);
value [len - d1 -1] = 0;
src[d1] = 0;



return 1;
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


