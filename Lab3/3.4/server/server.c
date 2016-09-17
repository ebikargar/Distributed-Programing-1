/*
 * server listening on a specified port, receives commands to retrieve files, and they are sent back to the sender
 */
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
#include "types.h"

#define LISTENQ 15
#define MAXBUFL 255

#ifdef TRACE
#define trace(x) x
#else
#define trace(x)
#endif

#define MSG_ERR "-ERR\r\n"
#define MSG_OK  "+OK\r\n"
#define MSG_QUIT  "QUIT\r\n"
#define MSG_GET "GET"

#define MAX_STR 1023

char *prog_name;


int receiver (int connfd) {

	char buf[MAXBUFL+1]; /* +1 to make room for \0 */
	//int op1, op2;
	//int res;
	int nread;
	int ret_val = 0;
	XDR 		xdrs_in;	/* input XDR stream */
	XDR		xdrs_out;	/* output XDR stream */
	message 	req;		/* request message */
	message 	res;	/* response message */
	unsigned int 	i;
	FILE *stream_socket_r;		/* FILE stream for reading from the socket */
	FILE *stream_socket_w;		/* FILE stream for writing to the socket */
	while (1) {
		trace( err_msg("(%s) - waiting for commands ...",prog_name) );
		/* open FILE streams for the socket anFILEd bind them to corresponding xdr streams */
		stream_socket_r = fdopen(connfd, "r");
		if (stream_socket_r == NULL)
			err_sys ("(%s) error - fdopen() failed", prog_name);
		xdrstdio_create(&xdrs_in, stream_socket_r, XDR_DECODE);

		stream_socket_w = fdopen(connfd, "w");
		if (stream_socket_w == NULL)
			err_sys ("(%s) error - fdopen() failed", prog_name);
		xdrstdio_create(&xdrs_out, stream_socket_w, XDR_ENCODE);

		trace( err_msg("(%s) - waiting for operands ...", prog_name) );

		/* receive request */
		req.tag = -1;
		req.message_u.filename = NULL;	/* xdr library will allocate memory */
		//req.message_u.fdata.last_mod_time = 0;
		//req.message_u.fdata.contents_len = 0;
		//req.message_u.fdata.contents_val = NULL; /* xdr library will allocate memory */

		/* send response */
		//res.tag = -1;
		//res.message_u.filename = NULL;	/* xdr library will allocate memory */
		//res.message_u.fdata.last_mod_time = 0;
		//res.message_u.fdata.contents.contents_len = 0;
		//res.message_u.fdata.contents.contents_val = NULL; /* xdr library will allocate memory */

		if (!xdr_message(&xdrs_in, &req)) {
	   	 xdr_destroy(&xdrs_in);
	    	return -1;
		}
		printf("Received request\n");
		
		
		if(req.tag == GET){
			char fname[MAX_STR+1];
			 if (req.message_u.filename==NULL){
	   		 free(req.message_u.filename);
			// free(res.message_u.fdata.contents.contents_val);
	   		 xdr_destroy(&xdrs_in);
	    		return -1;
			}
			strcpy(fname, req.message_u.filename);
			if(strcmp(fname, "QUIT") == 0){
			 free(req.message_u.filename);
			// free(res.message_u.fdata.contents.contents_val);
	   		 xdr_destroy(&xdrs_in);
			trace( err_msg("(%s) --- client asked to terminate connection", prog_name) );
			ret_val = 1;
			break;
			}
			strcpy(fname, req.message_u.filename);//+6?
			
			trace( err_msg("(%s) --- client asked to send file '%s'",prog_name, fname) );

			struct stat info;
			int ret = stat(fname, &info);
			if (ret == 0) {//means file exited on the server side
				FILE *fp;
				if ( (fp=fopen(fname, "rb")) != NULL) {
			/* send MSG_OK + filesize to client */
					unsigned int size = info.st_size;/*gives you size of the file*/
					/* NB: strlen, not sizeof(MSG_OK), otherwise the '\0' byte will create problems when receiving data */			res.tag = OK;
					/* encode and send response structure 
					if (!xdr_message(&xdrs_out, &res)) {
					printf("Encoding error\n");
					xdr_destroy(&xdrs_out);
					xdr_destroy(&xdrs_in);
					free(req.message_u.filename);
					free(res.message_u.fdata.contents.contents_val);
					return -1;
					}*/
					//fflush(stream_socket_w);
					//Write (connfd, MSG_OK, strlen(MSG_OK) );
					trace( err_msg("(%s) --- sent '%s' to client",prog_name, MSG_OK) );

					//uint32_t val = htonl(size);//write the file size into buffer
					res.message_u.fdata.contents.contents_len = size;
					trace( err_msg("(%s) --- sent '%u' - converted in network order - to client: size",prog_name, res.message_u.fdata.contents.contents_len) );

					unsigned int mtime=info.st_mtime; /*gives the file last modified time*/
					//uint32_t mtval = htonl(mtime);//write the last modification time into buffer
					res.message_u.fdata.last_mod_time = mtime;
					trace( err_msg("(%s) --- sent '%u' - converted in network order - to client:last modification",prog_name, res.message_u.fdata.last_mod_time) );

					char *fileContent;
					fileContent = (char *)malloc(size*sizeof(char));
					nread = fread(fileContent,1,info.st_size,fp);
					if(nread < size) {
					printf("---error in reading the file\n");
					return -1;
					}	
					//fclose(fp);
					res.message_u.fdata.contents.contents_val = (char *)malloc(size*sizeof(char));
					if (res.message_u.fdata.contents.contents_val==NULL){
	  				xdr_destroy(&xdrs_in);
					//xdr_destroy(&xdrs_out);
	   				free(req.message_u.filename);
	   			 	free(res.message_u.fdata.contents.contents_val);
					trace( err_msg("(%s) --- can't read file content!",prog_name) );
	    				return -1;
					}
					res.message_u.fdata.contents.contents_val = fileContent;
					

					/* encode and send response structure */
					if (!xdr_message(&xdrs_out, &res)) {
					printf("Encoding error\n");
					xdr_destroy(&xdrs_out);
					xdr_destroy(&xdrs_in);
					free(req.message_u.filename);
	    				free(res.message_u.fdata.contents.contents_val);
	    				//free(res.response.xdrhypersequence_val);
					return -1;
					}
					fflush(stream_socket_w);
					fclose(fp);
				} else {
					ret = -1;//file is not existed on the server or cannot be opened
				}
			}
			if (ret != 0) {	
					res.tag = ERR;

					/* encode and send response structure */
					if (!xdr_message(&xdrs_out, &res)) {
					printf("Encoding error\n");
					xdr_destroy(&xdrs_out);
					xdr_destroy(&xdrs_in);
					free(req.message_u.filename);
	    				free(res.message_u.fdata.contents.contents_val);
					return -1;
					}
					fflush(stream_socket_w);
			}
		} else if (req.tag == QUIT){
			trace( err_msg("(%s) --- client asked to terminate connection", prog_name) );
			ret_val = 1;
			break;
		} else {
					res.tag = ERR;

					/* encode and send response structure */
					if (!xdr_message(&xdrs_out, &res)) {
					printf("Encoding error\n");
					xdr_destroy(&xdrs_out);
					xdr_destroy(&xdrs_out);
					free(req.message_u.filename);
	    				free(res.message_u.fdata.contents.contents_val);
					return -1;
					}
					fflush(stream_socket_w);
			trace( err_msg("(%s) --- Error accured!!", prog_name) );
		}

	}
	return ret_val;
}


int main (int argc, char *argv[]) {

	int listenfd, connfd, err=0;
	short port;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddrlen = sizeof(cliaddr);

	/* for errlib to know the program name */
	prog_name = argv[0];

	/* for errlib to know the program name */
	prog_name = argv[0];

	/* check arguments */
	if (argc!=3 || strlen(argv[1])!=2 || argv[1][0]!='-' ||
			(argv[1][1]!='x'))
		err_quit ("usage: %s <protocol> <port>\n where <protocol> can be -a -x", prog_name);
	port=atoi(argv[2]);


	/* create socket */
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	/* specify address to bind to */
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl (INADDR_ANY);

	Bind(listenfd, (SA*) &servaddr, sizeof(servaddr));

	trace ( err_msg("(%s) socket created",prog_name) );
	trace ( err_msg("(%s) listening on %s:%u", prog_name, inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port)) );

	Listen(listenfd, LISTENQ);

	while (1) {
		trace( err_msg ("(%s) waiting for connections ...", prog_name) );

		connfd = Accept (listenfd, (SA*) &cliaddr, &cliaddrlen);
		trace ( err_msg("(%s) - new connection from client %s:%u", prog_name, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port)) );

		err = receiver(connfd);

		Close (connfd);
		trace( err_msg ("(%s) - connection closed by %s", prog_name, (err==0)?"client":"server") );
	}
	return 0;
}

