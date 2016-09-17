/*
 * client which connects to a server, asks for a file name given from a command line and displays it
 */
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
#include "types.h"

#define MAXBUFL 255
#define MAX_STR 1023

#define MSG_OK "+OK"
#define MSG_QUIT "QUIT"

#ifdef TRACE
#define trace(x) x
#else
#define trace(x)
#endif

char *prog_name;
int fcount=0;


int main (int argc, char *argv[]) {

	int sockfd;
	char *dest_h, *dest_p;
	struct sockaddr_in destaddr;
	struct sockaddr_in *solvedaddr;
	//int op1, op2, res, nconv;
	char buf[MAXBUFL];
	struct addrinfo *list;
	//int err_getaddrinfo;
	char *fname;//+1
	XDR 		xdrs_in;	/* input XDR stream */
	XDR		xdrs_out;	/* output XDR stream */
	message 	req;		/* request message */
	message 	res;		/* response message */
	unsigned int 	i;
	FILE *stream_socket_r;		/* FILE stream for reading from the socket */
	FILE *stream_socket_w;		/* FILE stream for writing to the socket */

	/* for errlib to know the program name */
	prog_name = argv[0];

	/* check arguments */
	
	if (argc!=4 || strlen(argv[1])!=2 || argv[1][0]!='-' ||
			(argv[1][1]!='x'))
		err_quit ("usage: %s <protocol> <host> <port>\n where <protocol> can be -a -x", prog_name);
	dest_h=argv[2];
	dest_p=argv[3];
	//fname=argv[3];

	Getaddrinfo(dest_h, dest_p, NULL, &list);
	solvedaddr = (struct sockaddr_in *)list->ai_addr;

	/* create socket */
	sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	/* specify address to bind to */
	memset(&destaddr, 0, sizeof(destaddr));
	destaddr.sin_family = AF_INET;
	destaddr.sin_port = solvedaddr->sin_port;
	destaddr.sin_addr.s_addr = solvedaddr->sin_addr.s_addr;

	trace ( err_msg("(%s) socket created",prog_name) );

	Connect ( sockfd, (struct sockaddr *)&destaddr, sizeof(destaddr) );

	trace ( err_msg("(%s) connected to %s:%u", prog_name, inet_ntoa(destaddr.sin_addr), ntohs(destaddr.sin_port)) );

	while(1){

		stream_socket_r = fdopen(sockfd, "r");
		if (stream_socket_r == NULL)
			err_sys ("(%s) error - fdopen() failed", prog_name);
		xdrstdio_create(&xdrs_in, stream_socket_r, XDR_DECODE);

		stream_socket_w = fdopen(sockfd, "w");
		if (stream_socket_w == NULL)
			err_sys ("(%s) error - fdopen() failed", prog_name);
		xdrstdio_create(&xdrs_out, stream_socket_w, XDR_ENCODE);

		trace( err_msg("(%s) - Please enter your requested filename:", prog_name) );
		/* create requerst (local representation ) */
		req.tag = -1;
		//req.message_u.filename = NULL;	/* xdr library will allocate memory */
		//req.message_u.fdata.last_mod_time = 0;
		//req.message_u.fdata.contents_len = 0;
		//req.message_u.fdata.contents_val = NULL; /* xdr library will allocate memory */

		/* received response from server */
		res.tag = -1;
		res.message_u.filename = NULL;	/* xdr library will allocate memory */
		res.message_u.fdata.last_mod_time = 0;
		res.message_u.fdata.contents.contents_len = 0;
		res.message_u.fdata.contents.contents_val = NULL; /* xdr library will allocate memory */
	//char filename[MAXBUFL];
	fname = (char *)malloc(MAXBUFL*sizeof(char));
	fgets(fname,MAXBUFL,stdin);
	fname[strlen(fname)-1]='\0';
	if( strcmp(fname,"QUIT")==0 ){
		//free(req.message_u.filename);
		req.tag = QUIT;
		/* encode and send response structure */
		if (!xdr_message(&xdrs_out, &req)) {
			printf("Encoding error\n");
			xdr_destroy(&xdrs_out);
			free(res.message_u.fdata.contents.contents_val);
			free(res.message_u.filename);
			free(fname);
			return -1;
			}
	
		fflush(stream_socket_w);
		trace( err_msg("(%s) --- client sent QUIT request!", prog_name) );		
    		//free(res.message_u.fdata.contents.contents_val);
		//free(res.message_u.filename);
		free(fname);
    		Close (sockfd);
        	return 0;    
   	 }
	
	trace( err_msg("(%s) - file name is %s with size of filename %d  \n", prog_name, fname, strlen(fname) ) );
	printf("file name is %s with size of filename %d  \n",fname , strlen(fname));
	req.message_u.filename = (char *)malloc(MAXBUFL*sizeof(char));
	req.message_u.filename = fname;
	
	 if (req.message_u.filename==NULL){
	   		 free(req.message_u.filename);
			 free(res.message_u.fdata.contents.contents_val);
			 free(res.message_u.filename);
			 free(fname);
	   		 //xdr_destroy(&xdrs_in);
	    		return -1;
			}
	

	req.tag = GET;

	if (!xdr_message(&xdrs_out, &req)) {
			printf("Encoding error\n");
			xdr_destroy(&xdrs_out);
			free(req.message_u.filename);
	    		free(res.message_u.fdata.contents.contents_val);
			free(res.message_u.filename);
			//free(fname);
			return -1;
			}
	
	fflush(stream_socket_w);
	free(req.message_u.filename);
	//free(req.message_u.filename);
	//free(res.message_u.fdata.contents.contents_val);
	//free(res.message_u.filename);
	trace ( err_msg("(%s) - data has been sent", prog_name) );

		if (!xdr_message(&xdrs_in, &res)) {
	   	 xdr_destroy(&xdrs_in);
		 xdr_destroy(&xdrs_out);
		free(res.message_u.fdata.contents.contents_val);
		free(res.message_u.filename);
		//free(fname);
	    	return -1;
		}
		printf("Received request\n");
		//trace( err_msg("(%s) --- received string '%s'",prog_name, res.tag) );

		if (res.tag == OK) {
			
			char fnamestr[MAX_STR+1];
			
			if (res.message_u.filename==NULL){
			
	   		 free(res.message_u.filename);
			 free(res.message_u.fdata.contents.contents_val);
	   		 xdr_destroy(&xdrs_in);
			 xdr_destroy(&xdrs_out);
			// free(fname);
	    		 return -1;
			}
			sprintf(fnamestr, "down_%d", fcount);
			fcount++;
		unsigned int file_bytes = res.message_u.fdata.contents.contents_len ;

		trace( err_msg("(%s) --- received file size '%u'",prog_name, file_bytes) );
		unsigned int last_modify = res.message_u.fdata.last_mod_time;
		trace( err_msg("(%s) --- received file last modification '%u'",prog_name, last_modify) );	

			FILE *fp;
			if ( (fp=fopen(fnamestr, "wb"))!=NULL) {

					char *fileContent;
					fileContent = (char *)malloc(file_bytes*sizeof(char));
					
					if (res.message_u.fdata.contents.contents_val==NULL){
	  				xdr_destroy(&xdrs_in);
					xdr_destroy(&xdrs_out);
	   				free(res.message_u.filename);
	   			 	free(res.message_u.fdata.contents.contents_val);
					free(fileContent);
					trace( err_msg("(%s) --- can't read file content!",prog_name) );
	    				return -1;
					}

					fileContent = res.message_u.fdata.contents.contents_val;
					int nwite = fwrite(fileContent,1,file_bytes,fp);
					if(nwite < file_bytes) {
					printf("---error in reading the file\n");
					free(fileContent);
					return -1;
					}	
				fclose(fp);
				free(fileContent);
				
				trace( err_msg("(%s) --- received and wrote file '%s'",prog_name, fnamestr) );

			} else {
				trace( err_msg("(%s) --- cannot open file '%s'",prog_name, fnamestr) );
			}
		} else {
			trace ( err_quit("(%s) - protocol error: received response ERR'%s'", prog_name) );
		}

	}
	
	/* At this point, the client can wait on Read , but if the server is shut down, the connection is closed and the Read returns */
	/* If the client is not waiting on Read, the server shut down does not affect the client */
	/* Read on a socket closed by the server immediately returns 0 bytes */
	/*	
	trace( err_msg("(%s) --- sleeping 5 sec",prog_name) );
	sleep(5);
	char d;
	trace( err_msg("(%s) --- reading 1 char",prog_name) );
	int nr = Read (sockfd, &d, sizeof(char));
	trace( err_msg("(%s) --- Read returned %d bytes",prog_name, nr) );
	*/	

	//Close (sockfd);

	xdr_destroy(&xdrs_in);
	xdr_destroy(&xdrs_out);
	free(res.message_u.filename);
	free(res.message_u.fdata.contents.contents_val);

	return 0;
}

