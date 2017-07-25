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

//Wait and signal functions
#define P(s) semop(s, &pop, 1)  
#define V(s) semop(s, &vop, 1) 
struct sembuf pop, vop ;

//queue data structure
typedef struct queue
{  				
    int request;                   
}queue;

int main(int argc,char*argv[])
{
	int i=0,j=0,z=0,qbuf,mutex,sz,sz1,up,randomtime,randomnumber,m=-1;
	FILE* ids=fopen("ids", "r");

	//creating a file if not present
	if(ids==NULL)
	{
		ids=fopen("ids", "w");
		fclose(ids);
		ids=fopen("ids", "r");
	}


	while(fscanf(ids,"%d %d %d %d %d",&qbuf,&mutex,&sz,&sz1,&up)<5)
	{
		if(z==0)
			printf("Server B not started yet...\n");
		z++;
	}

	fclose(ids);


	queue*q = (queue*) shmat(qbuf, 0, 0);
	int*k = (int*) shmat(sz, 0, 0);
	int*k1 = (int*) shmat(sz1, 0, 0);
	int *update = (int*) shmat(up, 0, 0);

	*update=0;

	while(1)
	{
		//Terminating process if B updates update variable
		if(*update==1)
		{	
			shmdt(q);
			shmdt(k);
			shmdt(k1);
			shmdt(update);
			shmctl(up, IPC_RMID, 0);
			printf("Cleaning up shared memory...\n");
			return 0;
		}
		randomtime=rand()%3;
		sleep(randomtime);
		randomnumber=rand()%11-5;
		m=(*k)%10;
		q[m].request=randomnumber;
		printf("Random number generated is %d\n",q[m].request);
		*k=(*k)+1;
		m=*k;

		//Checking for reading and writing index constraint
		while(m-*k1==9);
	}
	return 0;
}
