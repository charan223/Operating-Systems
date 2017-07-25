#ifndef THREADS_SIGNAL_H
#define THREADS_SIGNAL_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
#include "threads/thread.h"


#define SIG_CHLD 0
#define SIG_CPU 1
#define SIG_UNBLOCK 2
#define SIG_USER 3
#define SIG_KILL 4

#define SIG_IGN 5
#define SIG_DFL 6


#define SIG_BLK 7
#define SIG_UNBLK 8
#define SIG_SETMASK 9



extern  struct list all_list;
extern  struct list unblock_list;

typedef int tid_t;


int mysignal(int signum, int handler);
int kill(tid_t tid, int sig);
int sigemptyset(int *set);
int sigfillset(int *set);
int sigaddset(int *set, int signum);
int sigdelset(int *set, int signum);
int sigprocmask(int how, int *set, int *oldset);


int CHLD_handler(tid_t ,tid_t);
int KILL_handler(tid_t ,tid_t);
int CPU_handler(tid_t ,tid_t);
int UNBLOCK_handler(tid_t ,tid_t);
int USER_handler(tid_t ,tid_t);




#endif







