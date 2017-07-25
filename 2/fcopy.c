#include <stdio.h>                                                                 //Declaring Libraries
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h> 
#include <fcntl.h> 
#include <string.h>

int main(int argc,char*argv[])
{
	int k1=open(argv[1],O_RDONLY,0);                                               //Opening file1 with read only
	if(k1==-1)
	{
	    perror("file1 open failed\n");										       //Printing error if open is failed
	    exit(-1);
	}            

	int k2=open(argv[2],O_WRONLY|O_CREAT|O_TRUNC,0700);                                                       //Opening file2 with write only or create
	if(k2==-1)
	{
	    perror("file2 open failed\n");											   //Printing error if open is failed
	    exit(-1);
	} 	

	char buff[100];
    int pipe1[2],pipe2[2],rcheck=100,rcheck1=100,wcheck=100,wcheck1=100;

    pipe(pipe1);																	//Creating pipes
    pipe(pipe2);

    int id = fork ();																//Child process creation
    if (id ==  0)
    {
        close(pipe1[1]);															//Closing write of pipe1
        close(pipe2[0]);															//Closing read of pipe2

        while(rcheck1==100)
        {
        rcheck1=read(pipe1[0],buff,100);                                            // Reading from pipe1

        wcheck1=write(k2,buff,rcheck1);												//Writing to file2

        if(wcheck1==-1) 													        //write to pipe2;
        {
      	    if(write(pipe2[1],"1",1))
      	    {
      		    printf("child printing an error message and exiting\n");
      	    }
            exit(-1);
        }
        if((wcheck1<100)&&(wcheck1!=-1))
        {
            write(pipe2[1],"0",1);
            printf("File copied successfully\n");
            exit(-1);
        }
        write(pipe2[1],"0",1);
        }
        exit(-1);
    }

  else
    {
        close(pipe1[0]);															//Closing read of pipe1
        close(pipe2[1]);
        buff[0]='0';
        buff[1]='\0';
     	while(rcheck==100 && !strcmp(buff,"0"))
     	{

            rcheck=read(k1,buff,100);												// Reading from file1

            rcheck=write(pipe1[1],buff,rcheck); 										    //Writing to pipe1;

            if(!rcheck)
                break;
            read(pipe2[0],buff,1);											        //Reading from pipe2
			buff[1]='\0';      
            if(strcmp(buff,"1")==0) 
            	exit(-1);
        }
    }
	exit(-1);
}