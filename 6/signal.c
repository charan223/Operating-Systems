#include "threads/thread.h"
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>
#include "threads/flags.h"
#include "threads/interrupt.h"
#include "threads/intr-stubs.h"
#include "threads/palloc.h"
#include "threads/switch.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "threads/signal.h"
#ifdef USERPROG
#include "userprog/process.h"
#endif


struct list all_list;
struct list unblock_list;


int CHLD_handler(tid_t to,tid_t from)
{
        //printf("e_c_h\n");
	struct list_elem* e;
	struct thread* q;
	for (e = list_begin (&all_list); e != list_end (&all_list); e = list_next (e))
	{
		q = list_entry (e, struct thread, allelem);
		if(to==q->tid) break;	
	}
	if(e==list_end(&all_list)) 
	{
		return 0;
	}
	q->child_count--;
	printf("Total number of children created by X (till the signal is delivered)\
                  is %d and the number of children still alive is %d\n",q->total_child_count,q->child_count);
	return 1;
}

int KILL_handler(tid_t to, tid_t from)
{
	printf("Thread_id %d is killed by %d thread\n",to,from);
        thread_exit();
	return 1;
}
int CPU_handler(tid_t to,tid_t from)
{
	struct list_elem* e;
	struct thread* q;
	for (e = list_begin (&all_list); e != list_end (&all_list); e = list_next (e))
	{
		q = list_entry (e, struct thread, allelem);
		if(to==q->tid) break;	
	}
	if(e==list_end(&all_list)) 
	{
		return 0;
	}
	printf("Maximum lifetime set is %d\n",q->life_time);
	thread_exit();
	return 1;
}
int UNBLOCK_handler(tid_t to,tid_t from)
{
	struct list_elem* e;
	struct thread* q;
	for (e = list_begin (&all_list); e != list_end (&all_list); e = list_next (e))
	{
		q = list_entry (e, struct thread, allelem);
		if(to==(q->tid)) break;	
	}
	if(e==list_end(&all_list)) 
	{
                printf("Thread %d is not there\n",to);
		return 0;
	}
	if(q->status==THREAD_BLOCKED)
	{
                printf("Unblocking thread %d\n",to);
		thread_unblock (q);
	}
        else{

                printf("Thread %d is not blocked\n",to);
            } 
	return 1;
}


int USER_handler(tid_t to,tid_t from)
{
	printf("Signal is called by thread id %d to %d\n",from,to);
	return 1;
}





int mysignal(int signum,int handler)
{
	if(!(handler==SIG_DFL || handler ==SIG_IGN))
	{
		printf("Invalid Handler\n");
		return 0;
	}
	if(!(signum==SIG_CHLD || signum ==SIG_KILL || signum==SIG_CPU || signum ==SIG_UNBLOCK || signum==SIG_USER))
	{
		printf("Invalid signal\n");
		return 0;
	} 
	if(signum==SIG_KILL && handler==SIG_IGN)
	{
		printf("SIG_KILL cannot be ignored\n");
		return 0;
	}	
	struct thread *t=thread_current();
	t->signal_register[signum]=handler-5;  //ignore -- 0 ; default -- 1;
	//printf("curr_id = %d registered signal %d with %d\n",t->tid,signum,handler-5);
        if(t->signal_register[signum]==0){
                                   t->signal_queue[signum]=0;
                                         }

        return 1;

}
int kill(tid_t tid, int signum)
{
	struct list_elem *e;	
	struct thread * t, *q;
	if(!(signum ==SIG_KILL || signum ==SIG_UNBLOCK || signum==SIG_USER))
	{
		printf("Invalid signal\n");
		return 0;
	} 

	for (e = list_begin (&all_list); e != list_end (&all_list); e = list_next (e))
	{
		t = list_entry (e, struct thread, allelem);
		if((t->tid)==tid) break;	
	}

	if(e==list_end(&all_list)) 
	{
		printf("No thread is associated with given thread id\n");
		return 0;
	}


        if(signum==SIG_USER)
	{

                //printf("inside_kill_siguser\n");
		if((t->signal_mask & 16) == 1)return 0;
          //       printf("inside_thread %d sigreg %d\n",t->tid,t->signal_register[SIG_USER]);
                if(t->signal_register[SIG_USER]==0)return 0;
                t->signal_queue[SIG_USER]=thread_current()->tid;
		//printf("filled_signal_queue_with_SIG_USER\n");
                return 1;
	
        }

	else if(signum==SIG_UNBLOCK)
	{
                if((t->signal_mask & 8) == 1)return 0;
                if(t->signal_register[SIG_UNBLOCK]==0)return 0;
		for (e = list_begin (&unblock_list); e != list_end (&unblock_list); e = list_next (e))
		{
			q = list_entry (e, struct thread, ubelem);
			if(t->tid==q->tid) break;	
		}
		if(e==list_end(&unblock_list)) 
		{
   			list_push_back (&unblock_list, &t->ubelem);
                        //printf("pushed to unblock list\n");
		}
		return 1;
	}

	else if(signum==SIG_KILL)
	{
		if(t->parent == thread_current()->tid)
		{
			t->signal_queue[SIG_KILL]=thread_current()->tid;
			return 1;
		}
		else
		{
			printf("Given thread is not the direct child of current thread\n");
			return 0;
		}
	}

        
return 0;
	
}


int sigemptyset(int *set){

 if(set == NULL)return 0;

 int i=0; 
 for(i=0;i<5;i++)set[i]=0;

 return 1;

}


int sigfillset(int *set){
 
if(set == NULL)return 0;

 int i=0; 
 for(i=0;i<5;i++)set[i]=-1; //..........................................from whom the signals go from?

 return 1;
 
}

int sigaddset(int *set, int signum){

  if(set == NULL)return 0;
  set[signum] = -1;
  return 1;

}

int sigdelset(int *set, int signum){

if(set == NULL)return 0;
  set[signum] = 0;
  return 1;


}

int sigprocmask(int how, int *set, int *oldset){

int i;
struct thread* cur = thread_current();

 if(set == NULL){

         if(oldset == NULL)return 0;
         oldset = malloc(sizeof(int));
         *oldset = cur->signal_mask;
         return 1;

 }


 if( !(how == SIG_BLK || how == SIG_UNBLK || how == SIG_SETMASK ) ){
      
         printf("Invalid 'how' flag\n");
         return 0;
  }
 
 if (oldset != NULL){

    oldset = malloc(sizeof(int));
    *oldset = cur->signal_mask;

 }

 
 if(how == SIG_BLK){

  
                 cur->signal_mask =  cur->signal_mask | (*set);            

                     
 }

 else if(how == SIG_UNBLK){


                 cur->signal_mask =  cur->signal_mask & (15-(*set));  //16-set give bitwise negation till 4 LSB            

                      


 }
 else{

                 cur->signal_mask = (*set);            

                      
 } 

return 1;

}
