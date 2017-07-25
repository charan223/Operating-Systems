//R.Sri Charan Reddy
//14CS10037

//Commands:
//gcc shuffle.c -pthread
//./a.out < inputfile

//including libraries
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>


//thread structure
struct thread_data {
	int**a;
	int t_index;
	int k;
	int x;
	int count;
	int n;
	int p;
	int q;
};

//initializing global count to zero
int global_count1 = 0;
int global_count2 = 0;
//declaring mutex and cond variables
pthread_mutex_t cnt_mutex;
pthread_cond_t cnt_cond1;
pthread_cond_t cnt_cond2;

// Function executed by all threads created
//Loads struct variables into individual variables
//Runs loop 2k times for each thread for k row and k column shuffles
//count gets incremented for every row/column shuffle
//count = even => row shuffle
//count = odd  => column shuffle
//shuffling from row q to q+p in row shuffle and similarly in column shuffle
//pthread mutex lock for global variable synchronization
//pthread cond waiting till finish of row/column shuffles or release of last row/column shuffle
//mutex unlock for global variable release
//pthread cond signal for releasing row/column shuffles which were stopped before

void *start_routine(void *param)
{
	// Take out the parameter
	struct thread_data *t_param = (struct thread_data *) param;

	//Loading into variables
	int tid = (*t_param).t_index;
	int count = (*t_param).count;
	int k = (*t_param).k;
	int x = (*t_param).x;
	int n = (*t_param).n;
	int p=  (*t_param).p;
	int q=  (*t_param).q;
	int**a= (*t_param).a;


	int i,j,m,l,h;

	//Running for loop 2k times for each thread for k row and k column shuffles
	for(i=0;i<2*k;i++)
	{
		if(count%2==0)
		{
			//row shuffle
			for(j=q;j<q+p;j++)
			{
				l=a[j][0];
				for(m=0;m<n-1;m++)
				{
					a[j][m]=a[j][m+1];
				}
				a[j][n-1]=l;
			}
			//row shuffle

			pthread_mutex_lock(&cnt_mutex);
			global_count1++;
			while (global_count1%x!=0)
			pthread_cond_wait(&cnt_cond2, &cnt_mutex);
			count++;
			pthread_mutex_unlock(&cnt_mutex);
			pthread_cond_signal(&cnt_cond2);			
		}
		else
		{

			//column shuffle
			for(j=q;j<q+p;j++)
			{
				l=a[n-1][j];
				for(m=1;m<n;m++)
				{
					a[n-m][j]=a[n-m-1][j];
				}
				a[0][j]=l;
			}
			//column shuffle

			pthread_mutex_lock(&cnt_mutex);
			global_count2++;
			while (global_count2%x != 0)
			pthread_cond_wait(&cnt_cond1, &cnt_mutex);
			count++;
			pthread_mutex_unlock(&cnt_mutex);
			pthread_cond_signal(&cnt_cond1);
		}
	}
	pthread_exit(NULL);
}


int main(int argc,char*argv[])
{
	int n,i,j,k,m,x,no_of_threads, id,**a;

	//Reading elements
	printf("Give the value of n\n");
	scanf("%d",&n);
	a=(int**) malloc(sizeof(int*)*n);
	for(i=0;i<n;i++)
	{
		a[i]=(int*)malloc(sizeof(int)*n);
	}
	printf("Give elements of the matrix row wise\n");
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			scanf("%d",&a[i][j]);
		}
	}
	printf("Input matrix is:\n");
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			printf("%d ",a[i][j]);
		}
		printf("\n");
	}
	printf("Enter k and x respectively\n");
	scanf("%d%d",&k,&x);

	struct thread_data *param=(struct thread_data*)malloc(sizeof(struct thread_data)*x);
	
	m = n/x;

	// initialize mutex
	pthread_mutex_init(&cnt_mutex, NULL);
	pthread_cond_init(&cnt_cond1, NULL);
	pthread_cond_init(&cnt_cond2, NULL);

	//t_index stores thread number
	//a       stores the matrix
	//k       stores k
	//x       stores x
	//count   stores count
	//n       stores n
	//p       stores n/x => number of rows/columns each thread to shuffle
	//q       stores from which row/column thread should start the shuffle
	for(i=0; i<x; i++)
	{
		param[i].t_index = i;
		param[i].a=a;
		param[i].k=k;
		param[i].x=x;
		param[i].count=0;
		param[i].n=n;
		param[i].p=n/x;
	}
	param[0].q=0;
	for(i=1 ; i< x;i++)
	{
		param[i].q=param[i-1].q+param[i-1].p;
	}
	param[x-1].p = n-m*(x-1);

        pthread_t *thread_id=(pthread_t*)malloc(sizeof(pthread_t)*x);

	// Create x threads
	for(i=0; i<x; i++)
	{ 
		pthread_create(&thread_id[i], NULL, start_routine, (void *) &param[i]);
	}

	// wait for all threads to finish
	for(i=0; i<x; i++)
	{
		pthread_join(thread_id[i], NULL);
	}


	//Printing final output matrix
	printf("Output matrix is:\n");
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			printf("%d ",a[i][j]);
		}
	printf("\n");
	}

	//freeing malloced variables
	for(i=0;i<n;i++)
	{
		free(a[i]);
	}
	free(a);


	// clean up the mutex and condition variable
	pthread_mutex_destroy(&cnt_mutex);
	pthread_cond_destroy(&cnt_cond1);
	pthread_cond_destroy(&cnt_cond2);
	return 0;
}
