#include <stdio.h>                                                                 //Declaring Libraries
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

void sort(int*A,int l,int r)                                                       //bubble sort function
{
	int i,j,max;
	for(i=l;i<r;i++)
	{
		for(j=l;j<r-i;j++)
		{
			if(A[j]>A[j+1]) 
			{
				max=A[j];
				A[j]=A[j+1];
				A[j+1]=max;
			}
		}
	}
}

int main(int argc,char *argv[])
{
	int*A=(int*)malloc(sizeof(int)*1000);
	int i,j,k,next;

	FILE* fp;
	fp=fopen(argv[1],"r");														   //Reading the file
	if(fp==NULL)																   //Exiting if file is not present
	{
		printf("File is absent,Enter a new file name");
		exit(-1);
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
	printf("\n");

	sort(A,0,i-1); 																   //Calling sort function

	printf("Integers in sorted array\n");
	for(j=0;j<i;j++)                                                               //Printing integers in the sorted array
	{
		printf("%d ",A[j]);
	}
	printf("\n");
			                                                                       //Freeing variables
	free(A);

	return 0;
}