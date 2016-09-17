#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
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

	int recvfd;
	short port;
	struct sockaddr_in servaddr, cliaddr;

	/* for errlib to know the program name */
	prog_name = argv[0];

	/* check arguments */
	if (argc!=2)
		err_quit ("usage: %s <port>\n", prog_name);
	port=atoi(argv[1]);

	/* create socket */
	recvfd = Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	printf("socket created...\n");
	
	/* specify address to bind to */
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl (INADDR_ANY);

Bind(recvfd, (SA*) &servaddr, sizeof(servaddr));

	printf("socket binded...\n");

	while (1) {
		
	int recv_size,cliaddr_len;
	char buf[MAXBUFL+1]; 
	int sockfd;	
	cliaddr_len=sizeof(cliaddr);
	recv_size = Recvfrom(recvfd, buf, MAXBUFL, 0, (struct sockaddr *) &cliaddr, &cliaddr_len);

	buf[recv_size]='\0';

	printf("--- received string is: '%s'", buf );

	Sendto(recvfd, buf, recv_size, 0, (struct sockaddr *) &cliaddr, cliaddr_len);

}
return 0;
}
