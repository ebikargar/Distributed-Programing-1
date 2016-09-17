/*
 *  warning: this is just a sample server to permit testing the clients; it is not as optimized or robust as it should be
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

#include <rpc/xdr.h>

#include <string.h>
#include <time.h>

#include "errlib.h"
#include "sockwrap.h"

//#define SRVPORT 1500

#define LISTENQ 15
#define MAXBUFL 255

#define MSG_ERR "wrong operands\r\n"
#define MSG_OVF "overflow\r\n"

#define MAX_UINT16T 0xffff
//#define STATE_OK 0x00
//#define STATE_V  0x01

#ifdef TRACE
#define trace(x) x
#else
#define trace(x)
#endif

char *prog_name;


int prot_a (int connfd) {

	char buf[MAXBUFL+1]; /* +1 to make room for \0 */
	uint16_t op1, op2;
	uint32_t res;
	int nread;

	while (1) {
		trace( err_msg("(%s) - waiting for operands ...",prog_name) );
		if ((nread = Readline (connfd, buf, MAXBUFL)) == 0 )
			return 0;

		/* append the string terminator after CR-LF that is, \r\n (0x0d,0x0a) */
		buf[nread]='\0';
		trace( err_msg("(%s) --- received string %s",prog_name, buf) );

		/* get the operands */
		if (sscanf(buf,"%hu %hu",&op1,&op2) != 2) {
			trace( err_msg("(%s) --- wrong or missing operands",prog_name) );
			Write (connfd, MSG_ERR, strlen(MSG_ERR));
			continue;
		}
		trace( err_msg("(%s) --- operands %hu %hu",prog_name,op1,op2) );

		/* do the operation */
		res = op1 + op2;

		/* check for overflow */
		if (res > MAX_UINT16T) {
			trace( err_msg("(%s) --- sum overflow",prog_name) );
			Write (connfd, MSG_OVF, strlen(MSG_OVF));
			continue;
		}

		trace( err_msg("(%s) --- result of the sum: %lu", prog_name, res) );

		/* convert the result to a string */
		snprintf (buf, MAXBUFL, "%u\r\n", res);

		/* send the result */
		Write (connfd, buf, strlen(buf) );
		trace( err_msg("(%s) --- result just sent back", prog_name) );
	}
}


int prot_x (int connfd)
{
	XDR xdrs_r, xdrs_w;
	//char buf[MAXBUFL];
	//int nread;
	int op1 = 0;
	int op2 = 0;
	int res;

	//xdrmem_create(xdrs, buf, MAXBUFL, XDR_DECODE);
	FILE *stream_socket_r = fdopen(connfd, "r");
	if (stream_socket_r == NULL)
		err_sys ("(%s) error - fdopen() failed", prog_name);
	xdrstdio_create(&xdrs_r, stream_socket_r, XDR_DECODE);

	FILE *stream_socket_w = fdopen(connfd, "w");
	if (stream_socket_w == NULL)
		err_sys ("(%s) error - fdopen() failed", prog_name);
	xdrstdio_create(&xdrs_w, stream_socket_w, XDR_ENCODE);

	trace( err_msg("(%s) - waiting for operands ...", prog_name) );

	/* get the operands */
	if ( ! xdr_int(&xdrs_r, &op1) ) {
		trace( err_msg("(%s) - cannot read op1 with XDR", prog_name) );		
	} else {
		trace( err_msg("(%s) - read op1 = %d", prog_name, op1) );
	}
	
	if ( ! xdr_int(&xdrs_r, &op2) ) {
		trace( err_msg("(%s) - cannot read op2 with XDR", prog_name) );
	} else {
		trace( err_msg("(%s) - read op2 = %d", prog_name, op2) );
	}

	/* do the operation */
	res = op1 + op2;

	trace( err_msg("(%s) --- result of the sum: %d", prog_name, res) );

	/* send the result */
	xdr_int(&xdrs_w, &res);
	fflush(stream_socket_w);

	trace( err_msg("(%s) --- result just sent back", prog_name) );

	return 0;
}


int main (int argc, char *argv[]) {

	int listenfd, connfd, err=0;
	short port;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddrlen = sizeof(cliaddr);

	/* for errlib to know the program name */
	prog_name = argv[0];

	/* check arguments */
	if (argc!=3 || strlen(argv[1])!=2 || argv[1][0]!='-' ||
			(argv[1][1]!='a' && argv[1][1]!='x'))
		err_quit ("usage: %s <protocol> <port>\n where <protocol> can be -a -x", prog_name);
	port=atoi(argv[2]);

	/* create socket */
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	/* specify address to bind to */
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl (INADDR_ANY);

	Bind(listenfd, (SA*) &servaddr, sizeof(servaddr));

	trace ( err_msg("(%s) socket created",prog_name) );
	trace ( err_msg("(%s) listening on %s:%u", prog_name, inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port)) );

	Listen(listenfd, LISTENQ);

	while (1) {
		trace( err_msg ("(%s) waiting for connections ...", prog_name) );

		connfd = Accept (listenfd, (SA*) &cliaddr, &cliaddrlen);
		trace ( err_msg("(%s) - new connection from client %s:%u", prog_name, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port)) );

		switch (argv[1][1])
		{
			case 'a':
				err = prot_a(connfd);
				break;
			case 'x':
				err = prot_x(connfd);
				break;
			default:
				err_quit ("(%s) bug - unexpected case in a switch", prog_name);
		}

		Close (connfd);
		trace( err_msg ("(%s) - connection closed by %s", prog_name, (err==0)?"client":"server") );
	}
	return 0;
}

