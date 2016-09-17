#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFLEN	256
#define size	32
#define MSG_OK "+OK"
#define MSG_Quit  "QUIT\r\n"


int main( int argc, char *argv[] )
{
	char *prog_name;
	char     	   buf[BUFLEN];		/* Transmission buffer */
	char	 	   rbuf[BUFLEN];	/* Reception buffer */
	char 		   fname[28];
	char  		   request[size];
	int	           s,socket_desc;
	int		   result;
	int                port_no;
	struct sockaddr_in saddr;		/* Server address structure */
	
 	prog_name = argv[0];
 /* Check number of arguments */
   if (argc!=3) {
      printf("Error! Number of Arguments are not sufficient \n");
      exit(0);
   }

/* Define client addr and port */
    	port_no=atoi(argv[2]);
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family      = AF_INET;
	saddr.sin_port        = htons(port_no);
	

if (inet_pton(AF_INET , argv[1] , &saddr.sin_addr) <= 0)
 {
    printf("Error to convert the address\n");
    if (close(s) == -1)
      printf("Error to close the socket\n");
      exit(1);
 }
	//fname=argv[3];

	/* create the socket */
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == -1)
	{
	printf("Socket() failed");
	exit(1);
}
   	printf("Socket Created... \n");

	/* connect socket */
	result= connect ( s, (struct sockaddr *)&saddr, sizeof(saddr) );
	if (result == -1){
		printf(" Connection Failed...\n");
		exit(1);
	}
	printf("Connection Done.\n");

while(1){
	memset(request,0,32); 
	memset(buf,0,BUFLEN); 
	printf("Please Enter Your Request:\n");
	fgets (request, 32 , stdin);
        if (strcmp(request,"QUIT")==0){
	sprintf(buf, MSG_Quit, request);	
	write(s, buf, strlen(buf)); /*send QUIT request to the server */
	}else
	{
        sprintf(buf, "GET%s\r\n", request);	
	write(s, buf, strlen(buf)); /*send file request to the server */
	}
    	printf ("Send request(%s) to server ...\n",buf);

/*Recieve & store file */
        fd_set rset;
	struct timeval tv;
	FD_ZERO(&rset);
	FD_SET(s, &rset);
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	if (select(s+1, &rset, NULL, NULL, &tv) > 0) {
		int nread = 0;
		char c;
		do {
			int n = read(s, &c, sizeof(char));
                        if (n==0){ break; }
			rbuf[nread++]=c;
		   } while (c != '\n' && nread < BUFLEN ); /* read nread character by char until reach to end of block */
		
			rbuf[nread]='\0';      			/* close end of nread */
		while (nread > 0 && (rbuf[nread-1]=='\r' || rbuf[nread-1]=='\n')) {
			rbuf[nread-1]='\0';
			nread--;
		}
		if (nread >= strlen(MSG_OK) && strncmp(rbuf,MSG_OK,strlen(MSG_OK))==0) { //???
			
			char fnamestr[BUFLEN];
			sprintf(fnamestr, "down_%s", fname);
			int n = read(s, rbuf, 4); /*size of file 4*8=32 */
			
			uint32_t file_bytes = ntohl((*(uint32_t *)rbuf));
			
			FILE *fp;
			if ( (fp=fopen(fnamestr, "wb"))!=NULL) {
				char c;
				int i;
				for (i=0; i<file_bytes; i++) {
					read (s, &c, sizeof(char));
					fwrite(&c, sizeof(char), 1, fp);
				}
				fclose(fp);
				printf ("File Received Successfully...\n");
				printf ("Recieved file is %s size of the File is: %d \n" , fnamestr,file_bytes);
			} else {
			     printf("Can not open the file...\n");
			}
		} else {
			  printf("File not founded on server... \n");
		}

	} 
	else {
		printf("Timeout,waiting for an answer from server...\n");
	}
}
/*close the socket*/
close(s);
return 0;
                                                                                                                                 
}
