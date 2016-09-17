/*
 * Sviluppato da Enrico Masala <masala@polito.it> , Apr 2011
 * server listening on a specified port, receives commands to retrieve files, and they are sent back to the sender
 * Each client is served by a separate process, with fork()
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>

#include <string.h>
#include <time.h>

#include "errlib.h"
#include "sockwrap.h"

#define LISTENQ 15
#define MAXBUFL 255

#ifdef TRACE
#define trace(x) x
#else
#define trace(x)
#endif

#define MSG_ERR "-ERR\r\n"
#define MSG_OK  "+OK\r\n"
#define MSG_QUIT  "QUIT"
#define MSG_GET "GET"

#define MAX_STR 1023

char *prog_name;

char prog_name_with_pid[MAXBUFL];

#define MAX_CHILDREN 2
int n_children;


static void sig_chld(int signo) {
	err_msg ("(%s) info - sig_chld() called", prog_name);
	n_children--;
	trace ( err_msg("(%s) - n_children = %d", prog_name, n_children) );
	int status;
	int pid;
	while ( (pid = waitpid(-1, &status, WNOHANG) ) > 0) {
		trace ( err_msg("(%s) info - waitpid returned PID %d", prog_name, pid) );
		n_children--;
		trace ( err_msg("(%s) - n_children = %d", prog_name, n_children) );
	}
	return;
}


int receiver (int connfd) {

	char buf[MAXBUFL+1]; /* +1 to make room for \0 */
	int op1, op2;
	int res;
	int nread;
	int ret_val = 0;

	while (1) {
		trace( err_msg("(%s) - waiting for commands ...",prog_name) );
		if ((nread = Readline (connfd, buf, MAXBUFL)) == 0 )
			return 0;

		/* append the string terminator after CR-LF */
		buf[nread]='\0';
		while (nread > 0 && (buf[nread-1]=='\r' || buf[nread-1]=='\n')) {
			buf[nread-1]='\0';
			nread--;
		}
		trace( err_msg("(%s) --- received string '%s'",prog_name, buf) );

		/* get the command */
		if (nread > strlen(MSG_GET) && strncmp(buf,MSG_GET,strlen(MSG_GET))==0) {
			char fname[MAX_STR+1];
			strcpy(fname, buf+4);

			trace( err_msg("(%s) --- client asked to send file '%s'",prog_name, fname) );

			struct stat info;
			int ret = stat(fname, &info);
			if (ret == 0) {
				FILE *fp;
				if ( (fp=fopen(fname, "rb")) != NULL) {
					int size = info.st_size;
					/* NB: strlen, not sizeof(MSG_OK), otherwise the '\0' byte will create problems when receiving data */
					Write (connfd, MSG_OK, strlen(MSG_OK) );
					trace( err_msg("(%s) --- sent '%s' to client",prog_name, MSG_OK) );
					uint32_t val = htonl(size);
					Write (connfd, &val, sizeof(uint32_t));
					trace( err_msg("(%s) --- sent '%d' - converted in network order - to client",prog_name, size) );
					int i;
					char c;
					for (i=0; i<size; i++) {
						fread(&c, sizeof(char), 1, fp);
						Write (connfd, &c, sizeof(char));
					}
					trace( err_msg("(%s) --- sent file '%s' to client",prog_name, fname) );
					fclose(fp);
				} else {
					ret = -1;
				}
			}
			if (ret != 0) {	
				Write (connfd, MSG_ERR, strlen(MSG_ERR) );
			}
		} else if (nread >= strlen(MSG_QUIT) && strncmp(buf,MSG_QUIT,strlen(MSG_QUIT))==0) {
			trace( err_msg("(%s) --- client asked to terminate connection", prog_name) );
			ret_val = 1;
			break;
		} else {
			Write (connfd, MSG_ERR, strlen(MSG_ERR) );
		}

	}
	return ret_val;
}


int main (int argc, char *argv[]) {

	int listenfd, connfd, err=0;
	short port;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddrlen = sizeof(cliaddr);
	pid_t childpid;
	struct sigaction action;
	int sigact_res;
	

	/* for errlib to know the program name */
	prog_name = argv[0];

	/* check arguments */
	if (argc!=2)
		err_quit ("usage: %s <port>\n", prog_name);
	port=atoi(argv[1]);

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

        memset(&action, 0, sizeof (action));
        action.sa_handler = sig_chld;
        sigact_res = sigaction(SIGCHLD, &action, NULL);
        if (sigact_res == -1)
                err_quit("(%s) sigaction() failed", prog_name);

	n_children = 0;



/*---------------------------------------------------------------------------------------
  Copy this part in you server just replace the receiver function with your own function.
//	Dont forget to define the variables
----------------------------------------------------------------------------------------*/

	while (1) {
		if ( n_children >= MAX_CHILDREN) {
			trace ( err_msg("(%s) - maximum number of children reached: NO accept now", prog_name) );
			int status;
			/* wait for a children to terminate */
			int wpid = waitpid(-1, &status, 0);
		}

		trace( err_msg ("(%s) waiting for connections ...", prog_name) );

		connfd = Accept (listenfd, (SA*) &cliaddr, &cliaddrlen);
		trace ( err_msg("(%s) - new connection from client %s:%u", prog_name, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port)) );

		trace ( err_msg("(%s) - fork() to create a child", prog_name) );
		
		if ( (childpid = Fork()) == 0) {
			/* Child */
			int cpid = getpid();
			sprintf(prog_name_with_pid, "%s child %d", prog_name, cpid);
			prog_name = prog_name_with_pid;
			Close(listenfd);
			err = receiver(connfd);
			exit(0);
		} else {
			/* Parent */
			n_children++;
			trace ( err_msg("(%s) - n_children = %d", prog_name, n_children) );
		}

		//Close (connfd);
		//trace( err_msg ("(%s) - connection closed by %s", prog_name, (err==0)?"client":"server") );
	}
/*---------------------------------------------------------------------------------------
  Copy this part in you server just replace the receiver function with your own function.
//	Dont forget to define the variables
----------------------------------------------------------------------------------------*/
	return 0;
}

