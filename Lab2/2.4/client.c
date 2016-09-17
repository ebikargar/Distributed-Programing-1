#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include <rpc/xdr.h>

#define MAXBUFL 255

char *prog_name;

int main(int argc, char *argv[]) 
{
  int sockfd;
  int port_no;
  struct sockaddr_in saddr;
  int op1, op2,res;
  char bufin[MAXBUFL],bufout[MAXBUFL];
  int n;
  XDR xdrs;
  XDR xdrs_r;
  unsigned int xdr_len;

prog_name = argv[0]; 

/*check number of arguments*/
if (argc < 3) {
      printf("Error! Number of Arguments are not sufficient");
      exit(0);
   }

/*create socket*/
sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd == -1) {
    printf("Socket cannot be created():\n");
    return 1;
  }
/*address and port config*/
  memset(&saddr, 0, sizeof(saddr));  /*make socket addr zero*/
  port_no=atoi(argv[2]);
  saddr.sin_family = AF_INET; // IPv4
  saddr.sin_port = htons(port_no);
  
  if (inet_pton(AF_INET,argv[1], &saddr.sin_addr) <= 0) {
    printf("Error to convert the address\n");
    if (close(sockfd) == -1)
      printf("Error to close the socket:\n");
    return 1;
      }

/*connect*/  
if (connect(sockfd, (struct sockaddr*)&saddr, sizeof(struct sockaddr_in)) == -1) {
    printf("Error to connect to sokcet():\n");
    if (close(sockfd) == -1)
      printf("Error to close the socket():\n");
    return 1;
  }
  else {
  	printf("Connection sucessfully established :\n");
  }

/*main loop of program*/
while(1)
{
  bzero (bufout, 255);
  printf("Please insert the first operand to send:");
  scanf("%d", &op1);

  printf("Please insert the second operand to send:");
  scanf("%d", &op2);

/*create XDR type*/
  xdrmem_create(&xdrs, bufout, MAXBUFL, XDR_ENCODE);
  xdr_int(&xdrs, &op1);
  xdr_int(&xdrs, &op2);
  
  xdr_len = xdr_getpos(&xdrs); /*get th length of filled XDR buffer*/

  write(sockfd, bufout, xdr_len);
  xdr_destroy(&xdrs);

 bzero (bufin, 255);
/*receive result*/ //do we need select here??
  n=recv(sockfd,bufin,MAXBUFL, 0);
  
  if (n <0 )
  {
  printf("Error to recieve Data()\n");
  fflush(stdout);
  }
   xdrmem_create(&xdrs_r, bufin, MAXBUFL, XDR_DECODE);

  if ( ! xdr_int(&xdrs_r, &res) ) {  /*read the data and put it in res */
    printf("cannot read res with XDR ...\n");
  } 
  else {
    printf("Sum: %d\n", res);
      xdr_destroy(&xdrs);
      }
}
}
 /*if you want to do in once close the socket 
close (sockfd);
return 0;
}
}*/