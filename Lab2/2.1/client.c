#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "errlib.h"
#include "sockwrap.h"

#define BUFSIZE 31

int main(int argc, char *argv[]) 
{
  char *prog_name;
  int 	sockfd;
  int 	port_no;
  struct sockaddr_in	saddr, caddr;
  fd_set			cset;
  struct timeval		tval;
  int n,i;
  int	caddrlen=sizeof(caddr);
  int	saddrlen=sizeof(saddr);
  int len;
  char buf[BUFSIZE];

/*Check Number of Arg*/
   if (argc < 4) {
      printf("Error! Number of Arguments are not sufficient");
      exit(0);
   }
/*Create socket*/
  sockfd = socket(AF_INET, SOCK_DGRAM,0);
  if (sockfd == -1) {
    printf("Socket cannot be creating():\n");
    return 1;
  }
/*Address and port config*/
  prog_name = argv[0];
  memset(&caddr, 0, sizeof(caddr));
  saddr.sin_family = AF_INET;  
  port_no=atoi(argv[2]);
  saddr.sin_port = htons(port_no);
  
  if (inet_pton(AF_INET,argv[1], &saddr.sin_addr) <= 0) {
    printf("Error to convert the address\n");
    if (close(sockfd) == -1)
      printf("Error to close the socket...\n");
    return 1;
      }
while(1) 
{
	len = strlen(argv[3]);
	if (len>31){
	printf("Size of Msg is exceeded than 31 Characters");
	exit(0);
}
else
{
/*Send Msg to server*/
  int s= sendto(sockfd, argv[3],len,0, (struct sockaddr *) &saddr,saddrlen); 
  printf("waiting for response...\n");

/* Waiting for receving answer run 5 times*/
  for (i=0;i<5;i++) 
  {
      FD_ZERO(&cset);
      FD_SET(sockfd, &cset);
      tval.tv_sec = 3; 
      tval.tv_usec = 0;

/*Select*/
      n = select(sockfd+1, &cset, NULL, NULL, &tval);
      if (n>0)
      {

/*Data received,recv and print Msg*/
     int m=recvfrom(sockfd,buf, 31 ,0,(struct sockaddr *)&caddr,&caddrlen);
       if (m != -1)
      {
	       buf[31] = '\0';
         printf("Recieved response is:%s and %d bytes \n", buf , m );
         fflush(stdout);
         break;
      }
       else
         printf ("Error in receiving response\n") ;
      } 
       else
	printf("No response received after %d+1 seconds\n", (int)tval.tv_sec);
  }
}
 /*close the socket */
  close(sockfd);
  return 0;
}
}
