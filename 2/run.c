#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


int main()
{

    int id =fork();
int status;
    if(id==0)
{
    char *argv1[2];                                              
    argv1[0] = (char *)malloc(strlen("./shell")+1);
    strcpy(argv1[0],"./shell");
    argv1[1] = NULL;
    execvp(argv1[0],argv1 ); 										                                 
    perror("execvp failed:");	
    exit(1);
}
else
{
wait(&status);
exit(1);
}							
    exit(-1);
}
			
