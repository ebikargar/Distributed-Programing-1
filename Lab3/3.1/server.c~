#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#define Readblock	256

#define LISTENQ 	10
#define MAXBUFL 	1024


#define MSG_Error "-ERR\r\n"
#define MSG_Ok    "+OK\r\n"
#define MSG_Quit  "QUIT\r\n"
#define MSG_Get   "GET"

#define MAX_CHILDREN 	2

char *prog_name;
sem_t semaphore;

void sig_handler(int signo);
void file_transfer (int connfd);

int main (int argc, char *argv[])
{
	int listenfd, connfd, err=0;
	int pid;
	short port;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddrlen = sizeof(cliaddr);
	socklen_t servaddrlen = sizeof(servaddr);

	/* for errlib to know the program name */
	prog_name = argv[0];

	/* check arguments */
	if (argc!=2)
		printf ("usage: %s <port>\n where", prog_name);

	port = strtol (argv[1], NULL, 0);

	/* create socket */
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	/* specify address to bind to */
	memset(&servaddr, 0, servaddrlen);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl (INADDR_ANY);

	/* Bind socket */	
	bind(listenfd, (struct sockaddr *) &servaddr, servaddrlen);

	printf("socket created\n");
	printf ("listening...\n");

	listen(listenfd, LISTENQ);

	if(sem_init(&semaphore, 0, MAX_CHILDREN) == -1)
		printf("Unable to init semaphore\n");

	/* Register the father's signal handler */
	signal(SIGCHLD, sig_handler);

	/* Main loop */
	while (1)
	{
		sem_wait(&semaphore); //decrease semaphore
		printf("waiting for connections ...");
		connfd = accept (listenfd, (struct sockaddr *)  &cliaddr, &cliaddrlen); //listnefd is the socket for parent
		/* Create a new process */
		pid = fork();
		if(pid < 0)
		{
			/* fork() failed */
			printf("Cannot fork. Exiting\n");
		}
		else if(pid == 0)
		{
			/* Child */
			/* Do the service */
			file_transfer(connfd);
			printf("connection closed by server");
			close (connfd);
			return 0;
		}
		else
		{
			/* Parent */
			printf("The father closes the socket\n");
			close(connfd);
		}

	}
	return 0;
}

/* signal handler and file-request functions*/

void sig_handler(int sig)
{
	int stat;
	pid_t pid;

	while((pid = waitpid((pid_t)-1, &stat, WNOHANG)) > 0)
		sem_post(&semaphore); //increase sem number+1
}
void file_transfer (int connfd)
		{
			char   request[32];
			char   buf[MAXBUFL];
			char c;
			char   buff[Readblock]; 
			struct stat info;
			int n;
			int nread;

  while(1){
		/* Recieve request function*/
                    nread=0;
	            memset(request,0,32);
		    do {
			n=read(connfd,&c,sizeof(char));
			
			request[nread++]=c;
			} while (c!='\n' && nread<32);
			request[nread]='\0';

		/* Check the name of the file and close it's end*/
		  	while (nread > 0 && (request[nread-1]=='\r' || request[nread-1]=='\n')) {
					request[nread-1]='\0';
					nread--;
				}

		    if (request[0]=='G'  &&   request[1]=='E'  &&  request[2]=='T') {
		  	    char fname[28] ;
			    memset(fname,0,28);
		  	    strcpy(fname, request+3);
		  	    printf("nread is %d\n", nread);


                        printf("client asked to send file %s\n",fname); 
			
		    	fflush(stdout);

			/* Open the file that we want to transfer */
			FILE *fp = fopen(fname,"rb");
			if(fp==NULL)
			{
			    printf (MSG_Error);
			    exit(1);   
			} 
			 printf ("File opened successfully...\n");
			  			
		/* convert filesize in network order and sent to client*/
			    stat(fname,&info);
			    int size = info.st_size;
					uint32_t val = htonl(size);
					write (connfd, MSG_Ok, strlen(MSG_Ok) );
					write (connfd, &val, sizeof(size));
					
		/* Read data from file and send it to buffer*/
			while(1)
			{
			/* First read file in block of 256 bytes */
			    bzero(buff, sizeof(buff));
			    nread = fread(buff,1,Readblock,fp); //Read from file and put it in buffer just 1 character

			/* If read was success, send data. */
			    if(nread > 0)
			    {
				printf("Sending file...\n");
				write(connfd, buff, nread);
			    }
			    if (nread==0){
			    	printf(MSG_Ok);
			    	fclose(fp);
			    	break;
			    }
                      }
                      continue;
		 }		    		          
		/*user asked for Quit*/
			else if ( request[0]=='Q'  &&   request[1]=='U'  &&  request[2]=='I' &&  request[2]=='T')
				{
					printf("Quit request,close communication channel");
					close(connfd);
                                        break;					
				} 
			/*what check here?*/
			else {
     			    write (connfd, MSG_Error, strlen(MSG_Error));
			    close(connfd);
			    continue;
			}
}
return 0;
}
