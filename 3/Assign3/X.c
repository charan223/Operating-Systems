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

//defining wait and signal 
#define P(s) semop(s, &pop, 1)  
#define V(s) semop(s, &vop, 1) 
struct sembuf pop, vop ;

//defining database for storing details of students
typedef struct database
{
    char first_name[21];   			
    char last_name[21];   				
    int roll_no;                   
    float cgpa;                   
}database;

//main function
int main(int argc,char*argv[])
{

	//intialising variables
	int dbuf,i=0,j=0,mutex,sz,up;

	//getting shared memory id into dbuf using shmget
	if((dbuf = shmget(IPC_PRIVATE, sizeof(database)*100, 0777|IPC_CREAT))==-1)
	{
		perror("shmget: shmget failed");
		exit(1); 
	}
	

	//creating database pointer to 100 student fields
	database*db = (database*) shmat(dbuf, 0, 0);

	//getting semaphore id into mutex using semget
	if((mutex=semget(IPC_PRIVATE,1,0777|IPC_CREAT))==-1)
	{
		perror("semget:semget failed");
		exit(1);
	}

	//initialising semaphore value to zero
	if(semctl(mutex, 0, SETVAL, 0)==-1)
	{
		perror("semctl:semctl failed");
	}

	//creating shared memory to store number of student fields in the database
    if((sz = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT))==-1)
	{
		perror("shmget: shmget failed");
		exit(1); 
	}
	
	//creating an integer pointer to store no of students
	int*k = (int*) shmat(sz, 0, 0);

	//creating shared memory to store update variable
    if((up = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT))==-1)
	{
		perror("shmget: shmget failed");
		exit(1); 
	}
	
	//creating an integer pointer to store no of students
	int*update = (int*) shmat(up, 0, 0);

	(*update)=0;

	//writing ids into a file ids
	FILE* ids=fopen("ids", "w");
	fprintf(ids,"%d %d %d %d",dbuf,mutex,sz,up);
	fclose(ids);

	//if number of input arguments is not 2,print error
	if ( argc != 2 ) 
    {
        printf("No filename\n");
        return 0;
    }
	
	//opening file for reading and writing
	FILE *file = fopen( argv[1], "r+" );

	//error if couldnot open the file
    if ( file == NULL )
    {
    	printf( "Could not open file\n" );
    	return 0;
    }
    else
    {

		//scanning elements from file into database
        while  ( !feof(file) )
        {
          	fscanf(file, "%s%s%d%f", db[i].first_name, db[i].last_name, &db[i].roll_no, &db[i].cgpa);
           	i++;
        }

		//closing file
		fclose(file);
    }
 
	(*k)=i;

	while(1)
	{

		//sleeping for 5 seconds after every loop
		sleep(5);

		if((*update)==1)
		{
			P(mutex);
			(*update)=0;
			file=fopen(argv[1], "w");
	
			//error if couldnot open the file
    		if ( file == NULL )
    		{
    			printf( "Could not open file\n" );
    			return 0;
    		}
	
			//copying modified database into file
			for(j=0;j<(*k-1);j++)
			fprintf(file,"%s %s %d %f\n", db[j].first_name, db[j].last_name, db[j].roll_no, db[j].cgpa);

			//closing file
    		fclose( file );
			V(mutex);
		}
	}
	return 0;
}
