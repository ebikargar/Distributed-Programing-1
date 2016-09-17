/************* UDP CLIENT CODE *******************/

#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <string.h>
#include <time.h>

#include "errlib.h"
#include "sockwrap.h"

#define MAXBUFL 255

char *prog_name;

int main (int argc, char *argv[]) {

	int clientSocket, portNum, nBytes;
  	char buffer[MAXBUFL];
	char *dest_addr;
  	struct sockaddr_in dest_Addr;
 	socklen_t addr_size;


/* for errlib to know the program name */
	prog_name = argv[0];
	dest_addr=argv[1];
	portNum=atoi(argv[2]);


/* check arguments */
	if (argc!=3)
		err_quit ("usage: %s <dest_host> <dest_port>", prog_name);
	
 /*Create UDP socket*/
clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

 /*Configure settings in address struct*/
  memset(dest_Addr.sin_zero, '\0', sizeof dest_Addr.sin_zero); 
  dest_Addr.sin_family = AF_INET;
  dest_Addr.sin_port = htons(portNum);
  dest_Addr.sin_addr.s_addr = inet_addr(dest_addr);

 /*Initialize size variable to be used later on*/
 addr_size = sizeof(dest_Addr);	

while(1){
    printf("Type a MSG to send to server:\n");
    fgets(buffer,MAXBUFL,stdin);
    nBytes = strlen(buffer) + 1;
    buffer[nBytes]='\0';
    printf("You typed: %s",buffer);

/*Send message to server*/
    sendto(clientSocket,buffer,nBytes,0,(struct sockaddr *)&dest_Addr,addr_size);
    printf("MSG  %s has bee sent...\n"  ,buffer);
    memset(buffer,'\0', MAXBUFL);

/*Receive message from server*/
    if ( recvfrom(clientSocket, buffer, nBytes, 0, (struct sockaddr *) &dest_Addr, &addr_size) == -1)
        {
            printf("Error in recvfrom() function\n");
        exit(1);
        }

    printf("Received from server: %s\n",buffer);

  }
  close(clientSocket);
  return 0;
}
