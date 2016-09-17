#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string.h>
#include <time.h>

#include "errlib.h"
#include "sockwrap.h"

#define MAXBUFL 255
#define MAXBUFFN 255
#define Backlog 15


#define MSG_ERR "+ERR\r\n"
#define MSG_OK  "+OK\r\n"
#define MSG_QUIT "GET QUIT\r\n"
#define MSG_GET "GET"

char *prog_name;

/*function receive file*/
int receiver (int connfd) {

	char buf[MAXBUFL+1];

	while (1) {
			printf( "---Waiting for new commands from client...\n");
			int nread = 0;
			char c;
			do {
				int n = Read(connfd, &c, sizeof(char));
				if (n==1)
					buf[nread++]=c;
				else
					break;
			} while (c != '\n' && nread < MAXBUFL-1);
			if (nread == 0)
				return 0;

			buf[nread]='\0';
			while (nread > 0 && (buf[nread-1]=='\r' || buf[nread-1]=='\n')) {
				buf[nread-1]='\0';
				nread--;
			}

	/* get the command */
			if (nread > strlen(MSG_GET) && strncmp(buf,MSG_GET,strlen(MSG_GET))==0) {
				char fname[MAXBUFFN+1];
				strcpy(fname, buf+4);
				printf( "--- client asked for file '%s' \n", fname);
			
				struct stat info;
				int ret = stat(fname, &info);

				if (ret == 0) { //means file exited on the server side
					FILE *fp;
					if ( (fp=fopen(fname, "rb")) != NULL) {
					
					/* send MSG_OK + filesize to client */	
						int size = info.st_size; /*gives you size of the file*/
						Write (connfd, MSG_OK, strlen(MSG_OK) );
						uint32_t val = htonl(size);
						Write (connfd, &val, sizeof(size));

						int mtime=info.st_mtime; /*gives the file last modified time*/
						uint32_t mtval = htonl(mtime);
						Write (connfd, &mtval, sizeof(size));

						int i;
						char c;
						for (i=0; i<size; i++) {
							fread(&c, sizeof(char), 1, fp);
							Write (connfd, &c, sizeof(char));
						}
						printf("--- sent file '%s' to client \n", fname);
						fclose(fp);
					}
				else {
						ret = -1;
						printf("file is not existed on the server or cannot be opened \n");
				}
				if (ret != 0) {	
					Write (connfd, MSG_ERR, strlen(MSG_ERR) );
				}
			}

			else if (nread >= strlen(MSG_QUIT) && strncmp(buf,MSG_QUIT,strlen(MSG_QUIT))==0) {

				printf("--- client asked to QUIT connection \n");
				close(connfd);
				return 0;
			}
			else {
				Write (connfd, MSG_ERR, strlen(MSG_ERR) );
			}
		}
		return 0;
	}
}

/*program main*/
int main (int argc, char *argv[]) {

	int port_no,listenfd;
	int connfd;
	struct sockaddr_in saddr, caddr;
	socklen_t caddrlen = sizeof(caddr);

	prog_name = argv[0];

/* check arguments */
	if (argc!=2)
		printf(" Number of Arguments are not sufficent \n");
	
	port_no=atoi(argv[1]);

/* create socket */
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

/* specify address to bind to */
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port_no);
	saddr.sin_addr.s_addr = htonl (INADDR_ANY);

Bind(listenfd, (SA*) &saddr, sizeof(saddr)); //SA* is sockaddr_in saddr defined in sockwrap.h

	printf("socket created \n");
	printf("listening on :%u \n", ntohs(saddr.sin_port) );

/* Listen */
Listen(listenfd, Backlog);

	while(1)
	{
		    printf("Waiting for new connection from user\n");
			connfd = Accept (listenfd, (SA*) &caddr, &caddrlen); 
			printf("--- there is new connection from client %s:%u \n", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port) );
            fflush(stdout);
			receiver(connfd);
	}

	Close (connfd);
	return 0;
}