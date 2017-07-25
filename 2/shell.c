#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h> 
#include <stdint.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <limits.h>

#define BUFSIZE 1024

static char perms_buff[30]; 
int stdin_hd;

//Permissions for ls -l command
const char *get_perms(mode_t mode) 
{ 
    char ftype = '?'; 
    if (S_ISREG(mode)) ftype = '-'; 
    if (S_ISLNK(mode)) ftype = 'l'; 
    if (S_ISDIR(mode)) ftype = 'd'; 
    if (S_ISBLK(mode)) ftype = 'b'; 
    if (S_ISCHR(mode)) ftype = 'c'; 
    if (S_ISFIFO(mode)) ftype = '|';     
    sprintf(perms_buff, "%c%c%c%c%c%c%c%c%c%c %c%c%c", 
    ftype, 
    mode & S_IRUSR ? 'r' : '-', 
    mode & S_IWUSR ? 'w' : '-', 
    mode & S_IXUSR ? 'x' : '-', 
    mode & S_IRGRP ? 'r' : '-', 
    mode & S_IWGRP ? 'w' : '-', 
    mode & S_IXGRP ? 'x' : '-', 
    mode & S_IROTH ? 'r' : '-', 
    mode & S_IWOTH ? 'w' : '-', 
    mode & S_IXOTH ? 'x' : '-', 
    mode & S_ISUID ? 'U' : '-', 
    mode & S_ISGID ? 'G' : '-', 
    mode & S_ISVTX ? 'S' : '-'); 
    return (const char *)perms_buff; 
}

//Execute function 
int executeCommand(char currDirectory[],char* argVarriables[],int argCount,char** envp,int isOutFile,int isInFile)
{
                int i;
                char* token;

                if(argVarriables[0]==NULL)
                    return(1);

                if(strcmp(argVarriables[0],"pwd")==0)                                               //pwd cmd implementation
                    printf("%s\n",currDirectory);
                

                else if(strcmp(argVarriables[0],"mkdir")==0)                                        //mkdir cmd implementation
                {
                    if(argVarriables[1]==NULL)
                        printf("mkdir: missing operand\n");
                    else
                    {
                        int dirCnt=1;
                        while(argVarriables[dirCnt]!=NULL)
                        {
                            struct stat st = {0};
                            if(stat(argVarriables[dirCnt], &st) == -1)
                                mkdir(argVarriables[dirCnt], 0777);
                            else
                                printf("mkdir: directory %s already exists\n",argVarriables[dirCnt]);
                            dirCnt++;
                        }
                    }
                }
                else if(strcmp(argVarriables[0],"cp")==0)                                           //cp implementation
                {
                if(argVarriables[1]==NULL || argVarriables[2]==NULL)
                printf("mkdir: missing operand\n");
			    int f1,f2,readbytes,writebytes;
			    time_t t1,t2;
			    double t;
       		    struct stat fst1={0},fst2={0};
       		    if (stat(argVarriables[1],&fst1) != 0) { printf("stat() failed with errno %d\n",errno); return(1); }
       		    if (stat(argVarriables[2],&fst2) != 0) { printf("stat() failed with errno %d\n",errno); return(1); }
			    t1=(fst1.st_mtime);
			    t2=(fst2.st_mtime);
			    t=difftime(t1, t2);
			    f1 = open(argVarriables[1], O_RDONLY);
			    if (f1 == -1)
			    {
         		   perror ("file1 opening error\n");
           		   return(1);
   			    }
	
			    f2 = open(argVarriables[2],O_WRONLY |O_TRUNC|O_CREAT, 0777);
			    if (f2 == -1)
			    {
         		   perror ("file2 opening error\n");
           		   return(1);
   			    }
			    readbytes=0; writebytes=0;
			    char buf[100];
			    if(t>0)
			    {
				   readbytes=read(f1,buf,100);
				   if(readbytes==-1) perror("read call1 failed\n");
   				   while (readbytes>0)
   				   {
					   writebytes=write(f2,buf,readbytes);
					   char buf[100];
					   readbytes=read(f1,buf,100);
   				   }   	
   				   printf("Contents copied from %s to %s\n",argVarriables[1], argVarriables[2]);
                   return(1);
			    }
			    else
			    {
				   printf("Cannot be copied due to modification constraints\n");
                   return(1);
			    }
			    close(f1);
			    close(f2);				
                        
                }
                
                else if(strcmp(argVarriables[0],"rmdir")==0)      //rmdir cmd implementation
                {
                    if(argVarriables[1]==NULL)
                        printf("mkdir: missing operand\n");
                    else
                    {
                        int dirCnt=1;
                        while(argVarriables[dirCnt]!=NULL)
                        {
                            struct stat st = {0};
                            if(stat(argVarriables[dirCnt], &st) == -1)
                                printf("rmdir: %s: No such file or directory\n",argVarriables[dirCnt]);
                            else
                            {
                                int del=rmdir(argVarriables[dirCnt]);
                                if(del==-1)
                                    printf("rmdir: directory %s could not be deleted (not empty)\n",argVarriables[dirCnt]);
                            }
                            dirCnt++;
                        }
                    }
                }

                else if(strcmp(argVarriables[0],"cd")==0)         //cd command implementation
                {
                    char* defaultPath="/home/charan";
                    struct stat st = {0};
                    if(argCount==1)
                    {
                        argVarriables[argCount]=defaultPath;
                        argCount++;
                        argVarriables[argCount]=NULL;
                    }

                    if(stat(argVarriables[1], &st) == -1)
                        printf("cd: %s: No such file or directory\n",argVarriables[1]);
                    
                    else if(chdir(argVarriables[1])==-1)
                        printf("cd: directory could not be changed\n");
                }

                else if(strcmp(argVarriables[0],"ls")==0)        //ls cmd implementation
                {
                    int isL=0;
                    if(argCount>1 && strcmp(argVarriables[argCount-1],"-l")==0)
                    {
                        isL=1;
                        argVarriables[argCount-1]=NULL;
                        argCount--;
                    }

                    if(argCount==1)
                    {
                        DIR *dirp=opendir(".");
                        struct dirent *dp;
                        struct stat statbuf;
                        struct passwd *pwd;
                        struct group *grp;
                        struct tm *tm;
                        char datestring[256];

                        while((dp=readdir(dirp)) != NULL) 
                        {
                            if(isL==0 && dp->d_name[0]!='.')
                            {
                                if (stat(dp->d_name, &statbuf) == -1)
                                    continue;
                                if(isOutFile)
                                    printf("%s\n",dp->d_name);
                                else
                                    printf("%s\t",dp->d_name);
                            }
                            else if(isL==1 && dp->d_name[0]!='.')
                            {
                                if (stat(dp->d_name, &statbuf) == -1)
                                    continue;

                                /* Print out type, permissions, and number of links. */
                                printf("%10.10s", get_perms(statbuf.st_mode));
                                printf("%4d", (int)statbuf.st_nlink);

                                /* Print out owner's name if it is found using getpwuid(). */
                                if ((pwd = getpwuid(statbuf.st_uid)) != NULL)
                                    printf(" %-8.8s", pwd->pw_name);
                                else
                                    printf(" %-8d", statbuf.st_uid);

                                /* Print out group name if it is found using getgrgid(). */
                                if ((grp = getgrgid(statbuf.st_gid)) != NULL)
                                    printf(" %-8.8s", grp->gr_name);
                                else
                                    printf(" %-8d", statbuf.st_gid);
                
                                /* Print size of file. */
                                printf(" %9jd", (intmax_t)statbuf.st_size);
                                tm = localtime(&statbuf.st_mtime);

                                /* Get localized date string and print */
                                strftime(datestring, sizeof(datestring), nl_langinfo(D_T_FMT), tm);

                                printf(" %s %s\n", datestring, dp->d_name);   //directory entery name
                            }
                        }
                        closedir(dirp);
                        if(!isL)
                        printf("\n");
                    }
                    else
                    {
                        int dirCnt=1;
                        while(argVarriables[dirCnt]!=NULL)
                        {
                            struct stat st = {0};
                            if(stat(argVarriables[dirCnt], &st) == -1)
                            {
                                printf("ls: cannot access %s: No such file or directory\n",argVarriables[dirCnt]);
                                dirCnt++;
                                continue;
                            }
                            printf("%s:\n",argVarriables[dirCnt]);
                            DIR *dirp=opendir(argVarriables[dirCnt]);
                            struct dirent *dp;
                            struct stat statbuf;
                            struct passwd *pwd;
                            struct group *grp;
                            struct tm *tm;
                            char datestring[256];

                            while((dp=readdir(dirp)) != NULL) 
                            {
                                if(isL==0 && strcmp(dp->d_name, ".")!=0 && strcmp(dp->d_name, "..")!=0)
                                    printf("%s\t",dp->d_name);

                                else if(isL==1 && strcmp(dp->d_name, ".")!=0 && strcmp(dp->d_name, "..")!=0)
                                {
                                    /* Print out type, permissions, and number of links. */
                                    printf("%10.10s", get_perms(statbuf.st_mode));
                                    printf("%4d", (int)statbuf.st_nlink);

                                    /* Print out owner's name if it is found using getpwuid(). */
                                    if ((pwd = getpwuid(statbuf.st_uid)) != NULL)
                                        printf(" %-8.8s", pwd->pw_name);
                                    else
                                        printf(" %-8d", statbuf.st_uid);

                                    /* Print out group name if it is found using getgrgid(). */
                                    if ((grp = getgrgid(statbuf.st_gid)) != NULL)
                                        printf(" %-8.8s", grp->gr_name);
                                    else
                                        printf(" %-8d", statbuf.st_gid);

                                    /* Print size of file. */
                                    printf(" %9jd", (intmax_t)statbuf.st_size);
                                    tm = localtime(&statbuf.st_mtime);

                                    /* Get localized date string and print */
                                    strftime(datestring, sizeof(datestring), nl_langinfo(D_T_FMT), tm);

                                    printf(" %s %s\n", datestring, dp->d_name);   //directory entery name
                                }
                            }
                            closedir(dirp);
                            dirCnt++;
                            if(!isL)
                                printf("\n");
                        }
                    }
                }

                else if(strcmp(argVarriables[0],"exit")==0)  //exit command implementation
                    exit(0);

                else
                {
                    char* pathVal=getenv("PATH");            //finding value of the path
                    char* pathList[100];
                    pathList[0]=currDirectory;
                    
                    int pathItr=1,pathCount;
                    token = strtok(pathVal,":");
                    while(token!=NULL)
                    {
                        pathList[pathItr]=token;
                        token = strtok(NULL,":");
                        pathItr++;
                    }
                    pathList[pathItr]=NULL;
                    pathCount=pathItr;

                    pid_t pid2=fork();
                    if(pid2==0)                               //child process
                    {
                        pathItr=0;
                        while(pathItr<pathCount)              //iterating to find the file 
                        {                                     //in same directory or any of the directories specified by "PATH" env var 
                            char prog[1024];
                            strcpy(prog,pathList[pathItr]);
                            strcat(prog,"/");
                            strcat(prog,argVarriables[0]);
                            if(execvp(prog,argVarriables)==-1)
			  // { perror("error:"); exit(1);}
                            pathItr++;
                        }
                        exit(0);
                    }
                    else                                      //parent process
                    {
                        wait(NULL);
                    }    
                }
                return 0;
}


int main(int argc, char **argv, char** envp)
{
    char command[BUFSIZE];                                   //get command command
    char currDirectory[1024];
    int argCount;                                            //arg count
    char* argVarriables[100];
    int i,j;
    char* token;
	stdin_hd=dup(0);
    while(1)
    {
    	if (getcwd(currDirectory, sizeof(currDirectory))!= NULL)    
    	{
    		printf("%s>",currDirectory);
			stdin=fdopen(dup(stdin_hd),"r");
    		if(!fgets(command, BUFSIZE, stdin))             //get command and put it in command
        		break;                                      //if user hits CTRL+D

        	if(strcmp(command,"\n")==0)
        		continue;

        	int cmdLen=strlen(command);
        	if (command[cmdLen - 1] == '\n')
            	command[cmdLen - 1] = '\0';
	

            char outFileName[BUFSIZE],inFileName[BUFSIZE];
            int isOutFile=0,isInFile=0;
            FILE* outFile;
            FILE* inFile;
            int tempItr=0,h=0;

            int stdin_copy=dup(0);
            int stdout_copy=dup(1);

            while(command[tempItr]!='\0')
            {
                if(command[tempItr]=='>')
                {
                    isOutFile=1;
                    isInFile=0;
                    h=tempItr;
                    tempItr++;
                    break;
                }
                else if(command[tempItr]=='<')
                {
                    isOutFile=0;
                    isInFile=1;
                    h=tempItr;
                    tempItr++;
                    break;
                }
                tempItr++;
            }
            if(isOutFile)
            {
		        i=0;
		        j=0;
                while(command[tempItr]!='\0')
                {
                    if(command[tempItr-i]==' ')
                    {
                    tempItr++;
		            if(command[tempItr]=='\0'||command[tempItr]==' ') break;
                    outFileName[i]=command[tempItr];
                    i++;
		            }
                }

		        tempItr++;
		if(command[tempItr]=='<')
		{
			isInFile=1;
			tempItr=tempItr+2;
			while(command[tempItr]!='\0')
			{
                		inFileName[j]=command[tempItr];
				j++;
				tempItr++;
			}
		}
		inFileName[j]='\0';
                outFileName[i]='\0';
                command[h]='\0';
                outFile=fopen(outFileName,"w");
                int fd=fileno(outFile);
                if(fd!=-1)
                {               
                    close(1);
                    int newFd=dup(fd);
                }
                else
                {
                    printf("file %s couldn't be opened\n",outFileName);
                    continue;
                }
		close(fd);
                struct stat st={0};
                
		if(inFileName[0]!='\0'){
		if(stat(inFileName, &st) == -1)
                {
                    printf("cd: %s: No such file or directory\n",inFileName);
                    continue;
                }
                else
                {
                    inFile=fopen(inFileName,"r");
                    int fd=fileno(inFile);
                    if(fd!=-1)
                    {               
                        close(0);
                        int newFd=dup(fd);
                    }
                    else
                    {
                        printf("file %s couldn't be opened\n",inFileName);
                        continue;
                    }
		   close(fd);
                }
		}
            }

            else if(isInFile)
            {
                i=0;
		        j=0;
                while(command[tempItr]!='\0')
                {
                    if(command[tempItr-i]==' ')
                    {
                    tempItr++;
		            if(command[tempItr]=='\0'||command[tempItr]==' ') break;
                    inFileName[i]=command[tempItr];
                    i++;
		            }
                }

	           	tempItr++;
	           	if(command[tempItr]=='>')
	           	{
			     isOutFile=1;
			     tempItr=tempItr+2;
			     while(command[tempItr]!='\0')
			     {
                		outFileName[j]=command[tempItr];
				        j++;
				        tempItr++;
		         }
		        }

		outFileName[j]='\0';
                inFileName[i]='\0';
                command[h]='\0';
                struct stat st={0};
                if(stat(inFileName, &st) == -1)
                {
                    printf("cd: %s: No such file or directory\n",inFileName);
                    continue;
                }
                else
                {
                    inFile=fopen(inFileName,"r");
                    int fd=fileno(inFile);
                    if(fd!=-1)
                    {               
                        close(0);
                        int newFd=dup(fd);
                    }
                    else
                    {
                        printf("file %s couldn't be opened\n",inFileName);
                        continue;
                    }
		close(fd);
                }
		if(outFileName[0]!='\0'){
                outFile=fopen(outFileName,"w");
                int fd1=fileno(outFile);
                if(fd1!=-1)
                {               
                    close(1);
                    int newFd1=dup(fd1);
                }
                else
                {
                    printf("file %s couldn't be opened\n",outFileName);
                    continue;
                }
		close(fd1);
                }

            }

            /*handling the piping '|' between processes*/
            int j=0,flag,flag2=0,otherIp=0,otherOp=0;
            int ipcPipe[2];
            pipe(ipcPipe);
            char command2[BUFSIZE];
            int oneTimeProcess=0;
            
            while(command[j]!='\0')
            {
                flag=0;
                if(command[j]=='|')
                {
                    command[j]='\0';
                    flag=1;
                }
                j++;
                if(flag)
                {
                    int lame=j,lame2=0;
                    while(command[lame]==' ')
                        lame++;
                    while(command[lame]!='\0')
                    {
                        command2[lame2]=command[lame];
                        lame2++;
                        lame++;
                    }
		
                    command2[lame2]='\0';




                    if(fork())           //parent process
                    {
                        oneTimeProcess=0;
                        wait(NULL);
                        lame=0;
                        while(command2[lame]!='\0')
                        {
                            command[lame]=command2[lame];
                            lame++;
                        }
                        command[lame]='\0';
                        close(0);
                        int newFd=dup(ipcPipe[0]);
                        otherIp=1;
                        j=0;
                    }
                    else                   //child process
                    {
                        oneTimeProcess=1;
                        close(1);
                        int newFd=dup(ipcPipe[1]);
                        otherOp=1;
                        break;
                    }
                }
            }

            i=0;
            token = strtok(command," ");
            while(token!=NULL)
            {
            	argVarriables[i]=token;
            	token = strtok(NULL," ");
            	i++;
            }
            argVarriables[i]=NULL;
            argCount=i;
            int backGroundPro=0;
            if(strcmp(argVarriables[argCount-1],"&")==0)
            {
            	backGroundPro=1;
            	argVarriables[argCount-1]=NULL;
            	argCount--;
            }
            else if(argVarriables[argCount-1][strlen(argVarriables[argCount-1])-1]=='&')
            {
            	backGroundPro=1;
            	argVarriables[argCount-1][strlen(argVarriables[argCount-1])-1]='\0';
            }

            if(backGroundPro==1)      //background process to be created
            {
                pid_t pid=fork();
                if(pid==0)             //child process
                {
                    if(executeCommand(currDirectory,argVarriables,argCount,envp,isOutFile,isInFile))
                        continue;
                    exit(0);
                }   
            }
            else
            {
            	if(executeCommand(currDirectory,argVarriables,argCount,envp,isOutFile,isInFile))
                    continue;
            }
            
            if(isOutFile)
            {   fclose(outFile);    dup2(stdout_copy,1);  }
            else if(isInFile)
            {   fclose(inFile); dup2(stdin_copy,0);  }
            if(otherOp)
            {   close(1);   dup2(stdout_copy,1);  }
            if(otherIp)
            {   close(0);   dup2(stdin_copy,0);   }
            close(stdout_copy);
            close(stdin_copy);
            if(oneTimeProcess)
                exit(0);
        }
    }
	return 0;
}
