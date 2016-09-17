/*
 * Sviluppato da Enrico Masala <masala@polito.it> , Mar 2011
 * client which connects to a server, sends two integers in ASCII followed by \n and receives the sum of them
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

#define MAXBUFL 255

#ifdef TRACE
#define trace(x) x
#else
#define trace(x)
#endif

char *prog_name;



int main (int argc, char *argv[]) {

	int sockfd, err=0;
	char *dest_h, *dest_p;
	struct sockaddr_in destaddr;
	struct sockaddr_in *solvedaddr;
	uint16_t op1, op2, res, nconv;
	char buf[MAXBUFL];
	struct addrinfo *list; //to be used in Getaddrinfo function
	int err_getaddrinfo;


	/* for errlib to know the program name */
	prog_name = argv[0];

	/* check arguments */
	if (argc!=3)
		err_quit ("usage: %s <dest_host> <dest_port>", prog_name);
	dest_h=argv[1];
	dest_p=argv[2];

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

	printf("Please insert the first operand to send to the server: ");
	scanf("%hu", &op1);
	printf("Please insert the second operand to send to the server: ");
	scanf("%hu", &op2);

	sprintf(buf, "%hu %hu\n", op1, op2);
	
	Write(sockfd, buf, strlen(buf));

	trace ( err_msg("(%s) - data has been sent", prog_name) );

	Readline(sockfd, buf, MAXBUFL);
	
	nconv = sscanf(buf, "%hu", &res);
	if (nconv==1) {
		printf("Sum: %d\n", res);
	} else {
		printf("Message: %s\n", buf);
	}

	Close (sockfd);

	return 0;
}

