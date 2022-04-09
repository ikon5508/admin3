#ifndef shared_h
#define shared_h 1
#include <time.h>


#define name_holder 128
#define smbuff_sz 1000
#define mdbuff_sz 10000
#define lgbuff_sz 100000

struct _buffer {
char *p;
int len;
int max;
};
typedef struct _buffer buffer_t;


struct _stopwatch
{
struct timespec start, end;
int sec, msec;
};
typedef struct _stopwatch stopwatch_t;



void stopwatch_start (stopwatch_t *t);
void stopwatch_stop (stopwatch_t *t);
//void result (int *sec, int *ms);


void err_ctl (const int rslt, const char *msg);
void killme (const char *msg);
char *parse_line (char *dest, const char *src);
int split_value (char *, const char, char *);
void trim (char *totrim);




#endif
