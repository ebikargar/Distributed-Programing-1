#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

char *prog_name;

int main(int argc, char *argv[]) 
{
  int sockfd;
  int port_no;
  struct sockaddr_in saddr;

/*check number of arguments*/
   if (argc < 3) {
      printf("Error!Number of Arguments are not sufficient");
      exit(0);
   }

   /* for errlib to know the program name */
  prog_name = argv[0];
  
/*create socket*/
sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd == -1) {
    printf("Socket cannot be creating(): %s\n", strerror(errno));
    return 1;
  }
//address and port conf
  port_no=atoi(argv[2]);
  saddr.sin_family = AF_INET; // IPv4
  saddr.sin_port = htons(port_no);
  
  if (inet_pton(AF_INET,argv[1], &saddr.sin_addr) <= 0) {
    printf("Error to convert the address (a)\n");
    if (close(sockfd) == -1)
      printf("Error to close the socket: %s\n", strerror(errno));
    return 1;
      }
//connect  
if (connect(sockfd, (struct sockaddr*)&saddr, sizeof(struct sockaddr_in)) == -1) {
    printf("Error to connect to sokcet(): %s\n", strerror(errno));
    if (close(sockfd) == -1)
      printf("Error to close the socket(): %s\n", strerror(errno));
    return 1;
  }
  else {
  	printf("Connection sucessfully established...\n");
  }
 //close the socket 
if (close(sockfd) == -1) {
    printf("Error to close the socket(): %s\n", strerror(errno));
    return 1;
  }
  
  return 0;
}
