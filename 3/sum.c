#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#define P(s) semop(s, &pop, 1)  
#define V(s) semop(s, &vop, 1)  
int full,empty,mutex;
struct sembuf pop, vop ;


//struct type for circular buffer
typedef struct circular_buffer
{
    int buffer[20];   			  // valid data buffer
    int *buffer_end; 			  // end of valid data buffer
    int count;                    // number of items in the buffer
    int *head;                    // pointer to head
    int *tail;                    // pointer to tail
} circular_buffer;

void cb_init(circular_buffer *cb)
{
    cb->buffer_end = cb->buffer + 20;
    cb->count = 0;
    cb->head = cb->buffer;
    cb->tail = cb->buffer;
}

void cb_push_back(circular_buffer *cb, int item)
{
    if(cb->count == 20)
	{
		return;
    }
    *cb->head=item;
    cb->head = cb->head + 1;
    if(cb->head == cb->buffer_end)
        cb->head = cb->buffer;
    cb->count++;
}

int cb_pop_front(circular_buffer *cb)
{

    int item;
    if(cb->count == 0)
	{
		return;
    }

	item=*cb->tail;
    cb->tail = cb->tail + 1;
    if(cb->tail == cb->buffer_end)
        cb->tail = cb->buffer;
    cb->count--;
	return item;
}





int main()
{
	int sum,cbuf,*SUM,i,k,j,p,l,m,n,item;
	circular_buffer *cb;

	//getting shmid's for both circular buffer and sum	
	if((cbuf = shmget(IPC_PRIVATE, sizeof(circular_buffer), 0777|IPC_CREAT))==-1)
	{
		perror("shmget: shmget failed");
		exit(1); 
	}
	cb = (circular_buffer *) shmat(cbuf, 0, 0);
	cb_init(cb);

	if((sum = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT))==-1)
	{
		perror("shmget: shmget failed");
		exit(1); 
	}

	SUM = (int *) shmat(sum, 0, 0);
	*SUM=0;


	printf("Give the values of m\n");
	scanf("%d",&m);
	printf("Give the values of n\n");
	scanf("%d",&n);

	//creating semaphores
	full = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
	empty = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
	mutex = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
	semctl(full, 0, SETVAL, 0);
	semctl(empty, 0, SETVAL, 20);
	semctl(mutex, 0, SETVAL, 1);


	pop.sem_num = vop.sem_num = 0;
	pop.sem_flg = vop.sem_flg = 0;
	pop.sem_op = -1 ; vop.sem_op = 1 ;


	int id[m+n];
	//producers for loop
	for(j=0;j<m;j++)
	{
		id[j]=fork();
		if(id[j]==0)
		{
			for(i=1;i<(51);i++)
			{
				P(empty);
					P(mutex);
						cb_push_back(cb, i);
						printf("writing %d by producer %d\n",i,j+1);
					V(mutex);
				V(full);
			}
			exit(0);
		}
		
	}

	//consumers for loop
	for(p=0;p<n;p++)
	{
		id[m+p]=fork();
		if(id[m+p]==0)
		{
			while(1){
			if(*SUM>=m*25*51)
				{
					V(full);
					exit(1);
				}
			P(full);
			if(*SUM>=m*25*51)
				{
					V(full);
					exit(1);
				}
				P(mutex);
					item=cb_pop_front(cb);
					(*SUM)+=item;
					printf("reading %d by consumer %d\n",item,p+1);
				V(mutex);
			V(empty);	
			}
		}
	}

	//waiting for all child process,that of producers and consumers
	int status[m+n];
	for(p=0;p<m+n;p++)
		waitpid(id[p],&status[p],0);

	//printing sum
	printf("Value of sum is %d,value to be obtained is %d\n",(*SUM),(m*25*51));


	//Removing semaphores	
	semctl(full, 0, IPC_RMID, 0);
	semctl(empty, 0, IPC_RMID, 0);
	semctl(mutex, 0, IPC_RMID, 0);

	return 0;
}
