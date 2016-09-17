/*
 * Sviluppato da Enrico Masala <masala@polito.it> , Mar 2011
 * UDP server which echoes back the content of received packets
 * Send answers back only if there has been maximum 3 requests from a client. Maximum 10 clients are received.
 */
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
#define MAXCLIENTS 10

#ifdef TRACE
#define trace(x) x
#else
#define trace(x)
#endif

char *prog_name;

/* use sockaddr_storage instead of sockaddr since the former is guaranteed to be large enough to hold an IPv6 address if needed */
struct sockaddr_storage clients[MAXCLIENTS];
int npck[MAXCLIENTS];
int tot_clients;


int prot_text (int sockfd) {

	char buf[MAXBUFL+1]; /* To allow adding the \0 at the end for strings */
	int recv_size;
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;

	peer_addr_len = sizeof(struct sockaddr_storage); /* this value has to be re-initialized each time the Recvfrom is called, see man page */
	recv_size = Recvfrom(sockfd, buf, MAXBUFL, 0, (struct sockaddr *) &peer_addr, &peer_addr_len);
	buf[recv_size]='\0';
	trace( err_msg("(%s) --- received string '%s'",prog_name, buf) );
		
	/* Code that can be easily ported to IPv6 : use sockaddr_storage and inet_ntop */
	/************/
	in_port_t port;
	char text_addr[INET6_ADDRSTRLEN];
	port = ((struct sockaddr_in *)&peer_addr)->sin_port;
	Inet_ntop(AF_INET, &( ((struct sockaddr_in *)&peer_addr)->sin_addr), text_addr, sizeof(text_addr));
	/************/

	trace( err_msg("(%s) --- source IP address port %s %u", prog_name, text_addr, ntohs(port)) );
	
	int flag_blocked = 0;
	/* Check if address/port is already present in the list */
	int i;
	for (i=0; i<tot_clients; i++) {
		struct sockaddr_in *src = (struct sockaddr_in *)&peer_addr;
		struct sockaddr_in *el = (struct sockaddr_in *)&(clients[i]); //an array of address
		//if (src->sin_port == el->sin_port && src->sin_addr.s_addr == el->sin_addr.s_addr) {
		/* Port changes at every UDP packet: the case with TCP would be different */
		if (src->sin_addr.s_addr == el->sin_addr.s_addr)
			break;
	}

	if (i==tot_clients) {
		/* Address has not been found in the list,so insert the client and it's address & port into array+tot_clinet++ */
		if (tot_clients < MAXCLIENTS) {
			struct sockaddr_in *el = (struct sockaddr_in *)&(clients[tot_clients]);
			struct sockaddr_in *src = (struct sockaddr_in *)&peer_addr;
			el->sin_family = AF_INET;
			el->sin_port = src->sin_port;
			el->sin_addr = src->sin_addr;
			npck[tot_clients]=1;

			tot_clients++;	
			trace( err_msg("(%s) --- this source address has been added to the list", prog_name) );
		} else {
			trace( err_msg("(%s) --- cannot add more than %d elements to the list", prog_name, MAXCLIENTS) );
		}
	} else {
		/* Address has been found */
		if (npck[i]>=3) {
			flag_blocked = 1;
			trace( err_msg("(%s) --- this source address has already sent 3 or more packets -> blocked", prog_name) );
		} else {
			trace( err_msg("(%s) --- this source address has sent only %d packets: adding one", prog_name, npck[i]) );
			npck[i]++;
		}
	}


	if ( ! flag_blocked) {
		Sendto(sockfd, buf, recv_size, 0, (struct sockaddr *) &peer_addr, peer_addr_len);
	}
}


int main (int argc, char *argv[]) {

	int recvfd;
	short port;
	struct sockaddr_in servaddr;
	int i;

	/* for errlib to know the program name */
	prog_name = argv[0];

	/* check arguments */
	if (argc!=2)
		err_quit ("usage: %s <port>\n", prog_name);
	port=atoi(argv[1]);

	for (i=0; i<MAXCLIENTS; i++) {
		memset(&(clients[i]), 0, sizeof(struct sockaddr_storage));
		npck[i] = 0;
	}
	tot_clients = 0;

	/* create socket */
	recvfd = Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	/* specify address to bind to */
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl (INADDR_ANY);

	Bind(recvfd, (SA*) &servaddr, sizeof(servaddr));

	trace ( err_msg("(%s) socket created",prog_name) );
	trace ( err_msg("(%s) listening for UDP packets on %s:%u", prog_name, inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port)) );

	while (1) {
		trace( err_msg ("(%s) waiting for a packet ...", prog_name) );
	
		prot_text(recvfd);
	}

	return 0;
}

