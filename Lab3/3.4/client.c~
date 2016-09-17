#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include "own.h"


#define MSG_QUIT1 "QUIT"
       

#define IPSIZE 100
#define   TCP  0
#define  UDP   1


int main( int argc , char * argv[]  ){
      int sockfd , sd ;
      char buf_in[1024],buf_out[1024];
      struct sockaddr_in  serverAddr, clientAddr;
      socklen_t addr_size;
      int i,counter;
      FILE *fd;
      char filename[255] ;

if (argc!=3 || argv[1][0]!='-' ||(argv[1][1]!='x' )
		err_quit ("usage: %s <protocol> <port>\n where <protocol> can be -a -x", prog_name);

prog_name = argv[0];
serv_h = argv[1];
serv_p = argv[2];
      
/*Create socket*/
    	sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

/*Configure settings in address struct*/
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

/* Get and send the request */
while(1){

   printf("Please input requesting file Name:"); 
   memset( filename , '\0' , 1024 );
   fgets( filename , 1024 , stdin   );
   int n=strlen(filename);

   while (n > 0 && (filename[n-1]=='\r' || filename[n-1]=='\n')) {
	filename[n-1]='\0';
	n--;
   }
   printf ("\n (%s) is asked by client\n",filename);

/* if client asked for QUIT */
if( strcmp(filename,MSG_QUIT)==0 ){

	    xdrmem_create( &xdrs_out , buf_out , 1024 , XDR_ENCODE ); //create a buf Encode
	    if (!xdr_string(&xdrs_out, &MSG_QUIT,strlen(MSG_QUIT))) { //read the req in xdr data type and put it inside the buf
	        xdr_destroy(&xdrs_out);
	        return -1;
	    }

	    int len = xdr_getpos(&xdrs_out);
	    int n=send(sockfd, buf_out, len, 0);
	    if (n != len)
	    {
		printf("Write error\n");
		xdr_destroy(&xdrs_out);
		
	    }

  	    xdr_destroy(&xdrs_out);
            shutdown(sockfd, SHUT_WR);
            return 1;
 }
/* client asked for GET file */
else{	

	    xdrmem_create( &xdrs_out , buf_out , 1024 , XDR_ENCODE ); 
	    if (!xdr_string(&xdrs_out, &filename,strlen(filename))) { 
	        xdr_destroy(&xdrs_out);
	        return -1;
	    }

	    int len = xdr_getpos(&xdrs_out);
	    int n=send(sockfd, buf_in, len, 0);
	    if (n != len)
	    {
		printf("Write error\n");
		xdr_destroy(&xdrs_out);
		
	    }
	    xdr_destroy(&xdrs_out);


	printf ("Command  has been sent...\n");

/*  client waiting for response */	
	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	if ( select(sockfd+1, &rset , NULL, NULL, NULL) > 0 ) {
		int nread = 0; 
		char c;
       		char * recv_msg;

/*  client recieved a response */	
        read(sockfd, buf_in, 1024 );
   	xdrmem_create(&xdrs_in, buf_in , n, XDR_DECODE);
	/* decode request */
	if (!xdr_string(&xdrs_in, &recv_msg,1024)) {
	    xdr_destroy(&xdrs_in);
	    printf("Decoding Error");
	    return -1;
	}


		if (strncmp(recv_msg,MSG_OK,strlen(MSG_OK))==0) {
                        printf ("Message OK received Successfully\n");
                        fflush(stdout);
			char fnamestr[1024];
			sprintf( fnamestr , "down_%s", filename );
			int n = read( sockfd , buf_in , 4); 

			uint32_t file_bytes = ntohl((*(uint32_t *)buf_in));
			printf("File size=%u \n ", file_bytes );
                        fflush(stdout);

			Read(sockfd, buf_in, 4);
			uint32_t last_modify = ntohl((*(uint32_t *)buf_in));
			printf("File last modified on: '%u' \n", last_modify);

			FILE *fp;
			if ( (fp=fopen(fnamestr, "wb"))!=NULL){
				char c;
				int i;
				for (i=0; i<file_bytes; i++) {
					read (sd, &c, sizeof(char));
					fwrite(&c, sizeof(char), 1, fp);
				}
				fclose(fp);
				printf( "received and write file %s \n",fnamestr );
                                return 0;
			}else {
				printf( "cannot open file %s\n", fnamestr );
                                return -1;
			}
		 }else if (strncmp(recv_msg,MSG_ERR,strlen(MSG_ERR))==0) {
			   printf("---- Error Reported -----\n");
			   return -1;
                        }else{                   
   		           printf("---protocol error: received response %s\n", buf_in );
                           return -1;
                        }               
	}


/* file Transfer request */
   n=send_command_rec_file( TcpSocket ,  cmd  ,  "test"  );
   if (  n==0  ) {
           printf("File was transfered successfully\n");
           continue;
   }else 
   if (  n==1 ) {
          break;
   }else{
          //printf("Error"); 
          continue;
   }


}

          printf(" Connection Closed successfully\n");          
          close(TcpSocket);

return 0;
}
