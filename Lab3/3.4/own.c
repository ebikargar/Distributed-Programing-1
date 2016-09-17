#include  "own.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include     <rpc/xdr.h>	


void check_arg( int narg , int rnarg  ){
   if (narg < rnarg  ) {
     printf (" few number of argument\n  ");
     exit(1);
   }
}


int find_pattern( char source[1024] , char pattern[1024]  ){

    if (  strncmp (source,pattern,strlen(pattern)) == 0 )
      return 0;
    else
     return -1;
}

}


int send_file( FILE * fd , char filename[255] , int sd  ){


}

int rec_file(FILE * fd , char filename[255] , int sd  ){
}


int send_command_rec_file( int sd ,  char * cmd ,   char filename[1024] ){  //for client

        char buf[1024],buf_in[1024];
        char * rec_msg;
        int len;
        struct timeval tv;
        fd_set rset;
        XDR 		xdrs_in;	/* input XDR stream */
	XDR		xdrs_out;	/* output XDR stream */
        char  * MSG_GET1;


        if ( ! find_pattern (cmd,"QUIT")  ) {

            char * MSG_QUIT1;
            MSG_QUIT1 = "QUIT";
	    xdrmem_create( &xdrs_out , buf , 1024 , XDR_ENCODE ); //create a buf Encode type-xdrs_out is a pointer to first element of this buf
	    if (!xdr_string(&xdrs_out, &MSG_QUIT1,strlen(MSG_QUIT1))) { //read the req in xdr data type and put it inside the buf
	        xdr_destroy(&xdrs_out);
	        return -1;
	    }

	    len = xdr_getpos(&xdrs_out);
	    int n=send(sd, buf, len, 0);
	    if (n != len)
	    {
		printf("Write error\n");
		xdr_destroy(&xdrs_out);
		
	    }

  	    xdr_destroy(&xdrs_out);
            shutdown(sd, SHUT_WR);
            return 1;
        }
        else{	

	    xdrmem_create( &xdrs_out , buf , 1024 , XDR_ENCODE ); //create a buf Encode type-xdrs_out is a pointer to first element of this buf
	    if (!xdr_string(&xdrs_out, &cmd,strlen(cmd))) { //read the req in xdr data type and put it inside the buf
	        xdr_destroy(&xdrs_out);
	        return -1;
	    }

	    len = xdr_getpos(&xdrs_out);
	    int n=send(sd, buf, len, 0);
	    if (n != len)
	    {
		printf("Write error\n");
		xdr_destroy(&xdrs_out);
		
	    }
	    xdr_destroy(&xdrs_out);


	printf ("Command  has been sent\n");

	FD_ZERO(&rset);
	FD_SET(sd, &rset);
	if ( select(sd+1, &rset , NULL, NULL, NULL) > 0 ) {
		int nread = 0; char c;


        read(sd, buf_in, 1024 );
   	xdrmem_create(&xdrs_in, buf_in , n, XDR_DECODE);
	/* decode request */
	if (!xdr_string(&xdrs_in, &rec_msg,1024)) {
	    xdr_destroy(&xdrs_in);
	    printf("Decoding Error");
	    return -1;
	}


		if (strncmp(rec_msg,MSG_OK,strlen(MSG_OK))==0) {
                        printf ("Message OK received Successfully\n");
                        fflush(stdout);
			char fnamestr[1024];
			sprintf( fnamestr , "down_%s", filename );
			int n = read( sd , buf , 4);  // Reading 4 bytes 4*8 = 32 Bytes  
			uint32_t file_bytes = ntohl((*(uint32_t *)buf));
			printf("File size=%u \n ", file_bytes );
                        fflush(stdout);

			FILE *fp;
			if ( (fp=fopen(fnamestr, "wb"))!=NULL){
				char c;
				int i;
				for (i=0; i<file_bytes; i++) {
					read (sd, &c, sizeof(char));
					fwrite(&c, sizeof(char), 1, fp);
				}
				fclose(fp);
				printf( "received and wrote file %s \n",fnamestr );
                                return 0;
			}else {
				printf( "cannot open file %s\n", fnamestr );
                                return -1;
			}
		 }else if (strncmp(rec_msg,MSG_ERR,strlen(MSG_ERR))==0) {
			   printf("---- Error Reported -----\n");
			   return -1;
                        }else{                   
   		           printf("---protocol error: received response %s\n", buf );
                           return -1;
                        }               
	}
  }
}





int rec_command( int sd  ){

	char buf_out[1024],buf[1024]; /* +1 to make room for \0 */
	int op1, op2;
	int res;
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
               ++counter;
	int n = read( sd , buf , 1024 );
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

			struct stat info;
			int ret = stat(filename, &info);

			if (ret == 0) {
				FILE *fp;
				if ( (fp=fopen(filename, "rb")) != NULL) {
					int size = info.st_size;

                                            char * MSG_OK1;
					    MSG_OK1="+OK\r\n";
					    xdrmem_create(&xdrs_out, buf_out, 1024, XDR_ENCODE); //create a buf Encode type-xdrs_out is a pointer to first element of this buf
					    if (!xdr_string(&xdrs_out, &MSG_OK1,strlen(MSG_OK1))) { //read the req in xdr data type and put it inside the buf
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
                                            printf("(%s) sent" , MSG_OK1 );
                                            fflush(stdout);
					    xdr_destroy(&xdrs_out);
					printf("--- sent (%s) ---- to client\n",MSG_OK);
					uint32_t val = htonl(size);
					write (sd , &val, sizeof(size));
					int i;
					char c;
					for (i=0; i<size; i++) {
						fread(&c, sizeof(char), 1, fp);
						write (sd, &c, sizeof(char));
					}
					printf( "---- sent file (%s) --- client\n",filename);
                                        fflush(stdout);
					fclose(fp);
                                        continue; 
				} else {

				


                                            char * MSG_ERR1;
					    MSG_ERR1="-ERR\r\n";
    
					    xdrmem_create(&xdrs_out, buf_out, 1024, XDR_ENCODE); //create a buf Encode type-xdrs_out is a pointer to first element of this buf
					    if (!xdr_string(&xdrs_out, &MSG_ERR1,strlen(MSG_ERR1))) { //read the req in xdr data type and put it inside the buf
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
			} else{


                                            char * MSG_ERR1;
					    MSG_ERR1="-ERR\r\n";


					    xdrmem_create(&xdrs_out, buf_out, 1024, XDR_ENCODE); //create a buf Encode type-xdrs_out is a pointer to first element of this buf
					    if (!xdr_string(&xdrs_out, &MSG_ERR1,strlen(MSG_ERR1))) { //read the req in xdr data type and put it inside the buf
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
     

		} else if (strncmp(input,MSG_QUIT,strlen(MSG_QUIT))==0) {
			printf("--- client asked to terminate connection-----\n");
                        close(sd);
			return 0;
		} else {


                                            char * MSG_ERR1;
					    MSG_ERR1="-ERR\r\n";

					    xdrmem_create(&xdrs_out, buf_out, 1024, XDR_ENCODE); //create a buf Encode type-xdrs_out is a pointer to first element of this buf
					    if (!xdr_string(&xdrs_out, &MSG_ERR1,strlen(MSG_ERR1))) { //read the req in xdr data type and put it inside the buf
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

