#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char* argv[])
{
    char *argv1[5];                                                //shell command => xterm -hold -e ./sort1 charan.txt
    argv1[0] = (char *)malloc(strlen("xterm ")+1);
    argv1[1] = (char *)malloc(strlen("-hold ")+1);
    argv1[2] = (char *)malloc(strlen("-e ")+1);
	argv1[3] = (char *)malloc(256);  
    strcpy(argv1[0], "xterm ");
    strcpy(argv1[1],"-hold");
    strcpy(argv1[2],"-e");
	sprintf(argv1[3], "./sort1 %s", argv[1]);
	argv1[4] = NULL;
    execvp("xterm", argv1); 										//Starting a new process xterm                                  
	perror("execvp failed:");										//If execvp fails
    exit(-1);
}
			
