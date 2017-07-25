//Including Libraries
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

//Defining number of threads
#define NUM_THREADS 10

#define P(s) semop(s, &pop, 1)  
#define V(s) semop(s, &vop, 1) 
struct sembuf pop, vop ;

//Defining Queue
typedef struct queue
{  				
    int request;                   
}queue;


int Ticket=100;
int global_count=0;
int *update,*k,*k1;
queue*q;
int mutex,qbuf,sz,sz1,up;
pthread_t thread_id[NUM_THREADS];
pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
FILE* ids;

//book_ticket function
void *book_ticket(void *param)
{
	pthread_mutex_lock(&mutex1);
	global_count++;
	pthread_mutex_unlock(&mutex1);

	int x,z,randomtime,*y,a;
	y = (int*) param;
	x = (*y);

	pthread_mutex_lock(&mutex2);
	z=Ticket-x;
	if(z >=0)
	{
		if(z>100) Ticket =100;
		else Ticket=z;
		printf("Value of Ticket is %d\n",Ticket);
		randomtime=rand()%3;
		sleep(randomtime);
		a =1;
	}
	else
	{
		randomtime=rand()%3;
		sleep(randomtime);
		printf("Request cannot be fulfilled\n");
		a=0;
	}
	pthread_mutex_unlock(&mutex2);

	pthread_mutex_lock(&mutex1);
	global_count--;
	pthread_mutex_unlock(&mutex1);

	if(a==1) return (void*)1;
	else return (void*)0;
}

//signal function
void sigint(int a)
{	
	//updating update variable to terminate A
	*update=1;
        pthread_mutex_destroy(&mutex1);
        pthread_mutex_destroy(&mutex2);
	semctl(mutex, 0, IPC_RMID, 0);
	shmdt(q);
	shmctl(qbuf, IPC_RMID, 0);
	shmdt(k);
	shmctl(sz, IPC_RMID, 0);
	shmdt(k1);
	shmctl(sz1, IPC_RMID, 0);
	shmdt(update);
	printf("\nCleaning up thread mutex,shared memory,mutexes...\n");
	ids=fopen("ids", "w");
	exit(-1);
}

int main(int argc,char*argv[])
{
	//sigint to detect ^C signal
	signal(SIGINT, sigint);
	int i,z=0,id,no_of_threads,x,m=0;

	//creating shared memory and semaphores
	if((qbuf = shmget(IPC_PRIVATE, sizeof(queue)*10, 0777|IPC_CREAT))==-1)
	{
		perror("shmget: shmget failed");
		exit(1); 
	}
   	if((sz = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT))==-1)
	{
		perror("shmget: shmget failed");
		exit(1); 
	}
   	if((sz1 = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT))==-1)
	{
		perror("shmget: shmget failed");
		exit(1); 
	}
	if((mutex=semget(IPC_PRIVATE,1,0777|IPC_CREAT))==-1)
	{
		perror("semget:semget failed");
		exit(1);
	}

    	if((up = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT))==-1)
	{
		perror("shmget: shmget failed");
		exit(1); 
	}


	//intialising variables
	if(semctl(mutex, 0, SETVAL, 0)==-1)
	{
		perror("semctl:semctl failed");
	}
	q = (queue*) shmat(qbuf, 0, 0);
	k = (int*) shmat(sz, 0, 0);
	k1 = (int*) shmat(sz1, 0, 0);
	update = (int*) shmat(up, 0, 0);
	*k1=0;
	*k=0;
	pthread_mutex_init(&mutex1, NULL);
	pthread_mutex_init(&mutex2, NULL);
	//Storing ids in a file to open it from A.c
	ids=fopen("ids", "w");
	fprintf(ids,"%d %d %d %d %d",qbuf,mutex,sz,sz1,up);
	fclose(ids);


	i=0;
	while(1)
	{
		if(*k  > *k1)
		{
			//wait if threads>10
			if(global_count==10)
			{
				while(global_count>5);
			}
			m=(*k1)%10;
			pthread_create(&thread_id[i], NULL, book_ticket, (void *) &q[m].request);
			i++;
			if(i>=10) i=0;
			(*k1)=(*k1)+1;
		}	

	}
	
	return 0;
}
