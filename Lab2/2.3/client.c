#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string.h>
#include <time.h>

#include "errlib.h"
#include "sockwrap.h"

#define MAXBUFL 255
#define MAX_REC 1023

#define MSG_OK "+OK"
#define MSG_QUIT "QUIT"

char *prog_name;

int main (int argc, char *argv[]) {

	int sockfd;
	struct sockaddr_in caddr;
	struct sockaddr_in saddr;
	char buf[MAXBUFL];
	char fname[MAXBUFL];
	char *serv_h, *serv_p;

prog_name = argv[0];
serv_h = argv[1];
serv_p = argv[2];


/* create socket */
	sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

/* specify address to bind to */
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(atoi(serv_p));
	
	if (inet_pton(AF_INET,serv_h, &saddr.sin_addr) <= 0) {
		    printf("Error to convert the address\n");
		    Close(sockfd);
		    return 1;
      }

/* connect socket */
	Connect ( sockfd, (struct sockaddr *)&saddr, sizeof(saddr) );
	printf(" client connected to %s:%u \n", inet_ntoa(saddr.sin_addr), ntohs(saddr.sin_port) );
while(1){

	fgets(fname,MAXBUFL,stdin);
	fname[strlen(fname)-1]='\0';
	//printf("file name is %s with size %d  \n",fname , strlen(fname));

/* Get the request */
	memset(buf, 0, strlen(buf));
	if( strcmp(fname,MSG_QUIT)==0 ){
		sprintf(buf, "%s\r\n", fname);
    	Write(sockfd, buf, strlen(buf));
    	Close (sockfd);
        return 0;    
    }
	else
	sprintf(buf, "GET %s\r\n", fname);

    printf("buf is %s  size is %d \n",buf, strlen(buf));
	Write(sockfd, buf, strlen(buf));
	printf("file request has been sent...\n");

/* listen by Select for response */
	
	fd_set rset;
	struct timeval tv;
	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	if (select(sockfd+1, &rset, NULL, NULL, &tv) > 0)
	 {
		char c;
		int nread=0;
		do {
			Read(sockfd, &c, sizeof(char)); //here we read file name from sockefd
			buf[nread++]=c;
		} while (c != '\n' && nread < MAXBUFL-1);
		buf[nread]='\0';

		/*Removing CR and NL from end of the file name*/
		while (nread > 0 && (buf[nread-1]=='\r' || buf[nread-1]=='\n')) {
			buf[nread-1]='\0';
			nread--;
		}

		printf(" received string is: '%s' \n", buf);

		if (nread >= strlen(MSG_OK) && strncmp(buf,MSG_OK,strlen(MSG_OK))==0) 
		{
			char fnameRec[MAX_REC+1];
			sprintf(fnameRec, "down_%s", fname);
			int n = Read(sockfd, buf, 4); //here we read file size from sockfd

			uint32_t file_bytes = ntohl((*(uint32_t *)buf));
			printf("received file size is'%u' \n", file_bytes);


			Read(sockfd, buf, 4);
			uint32_t last_modify = ntohl((*(uint32_t *)buf));
			printf("File last modified on: '%u' \n", last_modify);


			FILE *fp;
			if ( (fp=fopen(fnameRec, "wb"))!=NULL) 
			{
				char c;
				int i;
				for (i=0; i<file_bytes; i++) {
					Read (sockfd, &c, sizeof(char));
					fwrite(&c, sizeof(char), 1, fp);
				}
				fclose(fp);
				printf(" received and wrote file '%s' \n",fnameRec);
			} 
			else {
				printf(" cannot open file '%s'", fnameRec);
			}
		}
		else {
			printf("file is not existed on the server side...\n");
		}
		
	}
	else {
		printf("No answer from server after second \n");
	}
}
Close (sockfd);

	return 0;
}