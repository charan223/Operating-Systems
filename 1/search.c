#include <stdio.h>                                                                 //Declaring Libraries
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int parid;                                                                         //Creating a global variable for main parent process id

void search(int*A,int l,int r,int k)                                               //Search function
{
	int i,id[2],status[2],mid=(l+r)/2;                                             //Declaring id and status                        

	if(r-l<=9)																	   //Linear search if number of elements are <=10
	{
		for(i=l;i<=r;i++)
		{
			if(A[i]==k)
			{
				printf("Element is found between the indices %d and %d\n",l,r);
				exit(1);
			}
		}

		if(getpid()!=parid) exit(0);											   //Exiting if it is not the main parent process
		else printf("Not found!!\n");											   //Printing status if not found in main parent process
	}
	else
	{
		id[0]=fork();                                                              //Calling First child
		if(id[0]==0)
		{
			search(A,l,mid,k);
		} 
		else
		{
			id[1]=fork();                                                          //Calling Second Child
			if(id[1]==0)
			{
				search(A,mid+1,r,k);
			}
			else
			{
				waitpid(id[0],&status[0],0);                                       //Waiting till the completion of First child
				waitpid(id[1],&status[1],0);                                       //Waiting till the completion of Second child
				printf("Status of first child is %d,Status of second child is %d\n",status[0],status[1]);
				if(parid==getpid())
				{
					if(	status[0]==256||status[1]==256) printf("Hurray!!Found\n"); //Printing if found 
					else printf("Not found!!\n");
				}
				else
				{
					if(	status[0]==256||status[1]==256) exit(1);
					else exit(0);
				}

			}
		
		}
	}
	exit(0);
}


int main()
{
	char*f=(char*)malloc(sizeof(char)*256);                                        //Creating variables required
	int*A=(int*)malloc(sizeof(int)*1000);
	int i,j,k,next;

	printf("Enter File Name of the File(less than 256 characters including extension)\n");
	scanf("%s",f);
	FILE*fp;
	fp=fopen(f,"r");															   //Reading the file
	while(fp==NULL)																   //Reading new file name,if old one is empty
	{
		printf("File is absent,Enter a new file name");
		scanf("%s",f);
		fp=fopen(f,"r");
	}

	for (i = 0; ; i++)                                                             //Reading integers into array
    {
        fscanf(fp, "%d", &A[i]);
        if(feof(fp)) break;														   //Checking End of File
    }

    printf("File is scanned and number of integers in the array is equal to %d\n",i);
    printf("And the integers are :\n");
    for(j=0;j<i;j++)                                                               //Printing integers in the array
	{
		printf("%d ",A[j]);
	}
	printf("\n");
    printf("Enter a key to search from the file\n");                               //Reading key 'k' to search
	scanf("%d",&k);

	parid=getpid();         													   //Storing main parent id

	search(A,0,i-1,k);                                                             //Calling search function

	free(f);                                                                       //Freeing variables
	free(A);

	return 0;
}
