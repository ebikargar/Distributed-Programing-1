#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include "own.h"


#define MSG_OK "+OK\r\n"
#define MSG_GET "+GET\r\n"
#define MSG_ERR "-ERR\r\n"



int rec_command( int connfd  ){

	char buf_out[1024],buf[1024];
	int ret_val = 0 , len ;
        int nread ; 
        int counter=-1;
        XDR 		xdrs_in;	/* input XDR stream */
	XDR		xdrs_out;	/* output XDR stream */
        char * input;

while (1) {
               memset( buf , 0 , 1024 );
               nread=0;
               char c;
		int n = read( connfd , buf , 1024 );
		xdrmem_create(&xdrs_in, buf, n, XDR_DECODE);
 	/* decode request */
	if (!xdr_string(&xdrs_in, &input,1024)) {
	    xdr_destroy(&xdrs_in);
	    printf("Decoding Error");
	    return -1;
	}


/* get the command */
		if ( strncmp(input,MSG_GET,strlen(MSG_GET))==0) {
			char filename[MAX_STR+1];
			strcpy(filename, input+3);

			printf( "---- client asked to send file (%s)----\n", filename );
/* get the status and last modification of the file */
			struct stat info;
			int ret = stat(filename, &info);


			if (ret == 0) {
				FILE *fp;
				if ( (fp=fopen(filename, "rb")) != NULL) {
					int size = info.st_size;

					    xdrmem_create(&xdrs_out, buf_out, 1024, XDR_ENCODE); 
					    if (!xdr_string(&xdrs_out, &MSG_OK,strlen(MSG_OK))) { //read the req in xdr data type and put it inside the buf
						xdr_destroy(&xdrs_out);
						return -1;
					    }

					    len = xdr_getpos(&xdrs_out);
					    n=send(connfd, buf_out, len, 0);
					    if (n != len)
					    {
						printf("Write error\n");
						xdr_destroy(&xdrs_out);
						
					    }
                                            printf("(%s) sent" , MSG_OK );
                                            fflush(stdout);
					    xdr_destroy(&xdrs_out);
					printf("--- sent (%s) ---- to client\n",MSG_OK);
/* Send the file size and last modification of the file */
					uint32_t val = htonl(size);
					write (connfd , &val, sizeof(size));
					
					int modify = info.st_mtime;
					uint32_t last_mod = htonl(modify);
					write (connfd , &last_mod, sizeof(size));

					int i;
					char c;
					for (i=0; i<size; i++) {
						fread(&c, sizeof(char), 1, fp);
						write (connfd, &c, sizeof(char));
					}
					printf( "---- sent file (%s) --- client\n",filename);
                                        fflush(stdout);
					fclose(fp);
                                        continue; 
				} else {	
  
					    xdrmem_create(&xdrs_out, buf_out, 1024, XDR_ENCODE); 
					    if (!xdr_string(&xdrs_out, &MSG_ERR,strlen(MSG_ERR))) { 
						xdr_destroy(&xdrs_out);
						return -1;
					    }

					    len = xdr_getpos(&xdrs_out);
					    n=send(connfd, buf_out, len, 0);
					    if (n != len)
					    {
						printf("Write error\n");
						xdr_destroy(&xdrs_out);
						
					    }
					    xdr_destroy(&xdrs_out);
                                        continue;    
					}
			} 
			else{
					    xdrmem_create(&xdrs_out, buf_out, 1024, XDR_ENCODE); 
					    if (!xdr_string(&xdrs_out, &MSG_ERR,strlen(MSG_ERR))) { //read the req in xdr data type and put it inside the buf
						xdr_destroy(&xdrs_out);
						return -1;
					    }

					    len = xdr_getpos(&xdrs_out);
					    n=send(connfd, buf_out, len, 0);
					    if (n != len)
					    {
						printf("Write error\n");
						xdr_destroy(&xdrs_out);
						
					    }
					    xdr_destroy(&xdrs_out);

                               continue;
                        }
     

		} else if (strncmp(input,MSG_QUIT,strlen(MSG_QUIT))==0) {
			printf("--- client asked to terminate connection-----\n");
                        close(sd);
			return 0;
		} else {
					    xdrmem_create(&xdrs_out, buf_out, 1024, XDR_ENCODE); //create a buf Encode type-xdrs_out is a pointer to first element of this buf
					    if (!xdr_string(&xdrs_out, &MSG_ERR,strlen(MSG_ERR))) { //read the req in xdr data type and put it inside the buf
						xdr_destroy(&xdrs_out);
						return -1;
					    }

					    len = xdr_getpos(&xdrs_out);
					    n=send(sd, buf_out, len, 0);
					    if (n != len)
					    {
						printf("Write error\n");
						xdr_destroy(&xdrs_out);
						
					    }
					    xdr_destroy(&xdrs_out);


                        continue;
		}

	}
 
}

/*Main Program*/
int main( int argc , char * argv[]  ){
      int sockfd , connfd ,port_no ;
      char buffer[1024];
      struct sockaddr_in  serverAddr, clientAddr;
      socklen_t addr_size;
      int i,counter;
      FILE *fd;
      char filename[255] ;
      

/* check arguments */
	if (argc!=2)
		printf(" Number of Arguments are not sufficent \n");
	
	port_no=atoi(argv[1]);

/* create socket */
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

/* specify address to bind to */
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port_no);
	serverAddr.sin_addr.s_addr = htonl (INADDR_ANY);

Bind(listenfd, (SA*) &serverAddr, sizeof(serverAddr)); //SA* is sockaddr_in saddr defined in sockwrap.h

	printf("socket created and binded... \n");
	printf("listening on :%u \n", ntohs(serverAddr.sin_port) );

/* Listen */
Listen(listenfd, Backlog);

  
	while(1)
	{
		    printf("Waiting for new connection from user...\n");
			connfd = Accept (listenfd, (SA*) &clientAddr, &clientAddr); 
			printf("--- there is new connection from client %s:%u \n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port) );
            		fflush(stdout);
			rec_command(connfd);
	}

	Close (connfd);
	return 0;
}
