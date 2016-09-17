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

char *prog_name;

int main (int argc, char *argv[]) {

	int sockfd, err=0;
	char *dest_h, *dest_p;
	char *message;
	struct sockaddr_in destaddr;
	socklen_t destaddr_len;	
	struct sockaddr_in *solvedaddr;
	char buf[MAXBUFL];
	struct addrinfo *list;
	int err_getaddrinfo;
	int recv_size;

/* for errlib to know the program name */
	prog_name = argv[0];

	/* check arguments */
	if (argc!=4)
		err_quit ("usage: %s <dest_host> <dest_port> <message>", prog_name);
	dest_h=argv[1];
	dest_p=argv[2];
	message=argv[3];

if ( strlen(message)>31)
exit(0);

	Getaddrinfo(dest_h, dest_p, NULL, &list);
	solvedaddr = (struct sockaddr_in *)list->ai_addr;

	/* create socket */
	sockfd = Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	printf("socket created...\n");

/* specify address to bind to socket*/
	memset(&destaddr, 0, sizeof(destaddr));
	destaddr.sin_family = AF_INET;
	destaddr.sin_port = solvedaddr->sin_port;
	destaddr.sin_addr.s_addr = solvedaddr->sin_addr.s_addr;

	Connect ( sockfd, (struct sockaddr *)&destaddr, sizeof(destaddr) );
	Send(sockfd, message, strlen(message), 0);

	printf (" MSG has been sent\n" );

	destaddr_len = sizeof(struct sockaddr);
	recv_size = Recvfrom ( sockfd, buf, MAXBUFL, 0, (struct sockaddr *)&destaddr, &destaddr_len );

        buf[recv_size]='\0';

	printf("Received string is: %s\n", buf);

	Close (sockfd);

	return 0;
}
