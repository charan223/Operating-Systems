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

//defining struct database and its fields
typedef struct database
{
    char first_name[21];   			
    char last_name[21];   				
    int roll_no;                   
    float cgpa;                   
} database;

//main function
int main(int argc,char*argv[])
{
	float gpa;
	int roll,dbuf,mutex,i,sz,up,option,z=0;

	FILE* ids=fopen("ids", "r");

	//If Y starts first,there will be no file,so creating it
	if(ids==NULL)
	{
		ids=fopen("ids", "w");
		fclose(ids);
		ids=fopen("ids", "r");
	}

	//If any copy of Y is started before X is started, it should wait for X to be started first
	while(fscanf(ids,"%d %d %d %d",&dbuf,&mutex,&sz,&up)<4)
	{
		if(z==0)
			printf("Waiting for X to start\n");
		z++;
	}

	fclose(ids);

	//creating database pointer to 100 student fields
	database*db = (database*) shmat(dbuf, 0, 0);

	//creating size of database
	int*k = (int*) shmat(sz, 0, 0);

	//creating size of database
	int*update = (int*) shmat(up, 0, 0);

	printf("Options:\n");
	printf("Enter 1 to show details of a student\n");
	printf("Enter 2 to update cgpa of a student\n");
	printf("Enter 3 to exit\n");

	//reading roll numbers from user
	printf("\nEnter option\n");
	scanf("%d",&option);

	//running while loop till exit(3) is given
	while(option!=3)
	{
		if(option==1)
		{
			printf("\nEnter roll number:\n");
			scanf("%d",&roll);
			for(i=0;i<(*k-1);i++)
			{
				if(db[i].roll_no==roll)
				{
					//showing the data of that particular student in which cgpa is updated
				  	printf("%s %s %d %f\n", db[i].first_name, db[i].last_name, db[i].roll_no, db[i].cgpa);
					break;
				}
			}		
		}
		else if(option==2)
		{
			P(mutex);
			V(mutex);
			printf("\nEnter roll number:\n");
			scanf("%d",&roll);
			for(i=0;i<(*k-1);i++)
			{
				if(db[i].roll_no==roll)
				{
					printf("\nEnter new CGPA:\n");
					scanf("%f",&gpa);
					db[i].cgpa=gpa;
					(*update)=1;
					break;
				}
			}
		}

		printf("\nEnter option:\n");
		scanf("%d",&option);
	}
	return 0;
}
