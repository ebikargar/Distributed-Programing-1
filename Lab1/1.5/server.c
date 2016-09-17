/************* UDP SERVER CODE *******************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

#define MAXBUFL 1024

int main(int argc, char *argv[]){

 char *prog_name; 
 int udpSocket, nBytes,port;
 char buffer[MAXBUFL];
 struct sockaddr_in serverAddr, dest_Addr;
 socklen_t server_addr_size, dest_Addr_size;
 int i;

/* check arguments */
	if (argc!=2) {
		printf ("Number of Arguments are not Sufficient\n");
	exit(0);
}
prog_name = argv[0];
port=atoi(argv[1]);

 /*Create UDP socket*/
  udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	printf("socket created...\n");

/*Configure settings in address struct*/

  memset((char *) &serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

/*Bind socket with address struct*/
  bind(udpSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	printf("socket is bind...\n");

 while(1){
     printf("Waiting for data...");
     fflush(stdout);

/* Try to receive any incoming UDP datagram */
 if ((nBytes = recvfrom(udpSocket, buffer, MAXBUFL, 0, (struct sockaddr *) &dest_Addr, &dest_Addr_size)) == -1)
        {
            printf("recvfrom() has error \n");
		exit(1);
        }
	printf("Data: %s\n" , buffer);

/*Convert message received to uppercase*/
    for(i=0;i<nBytes-1;i++)
      buffer[i] = toupper(buffer[i]);

/*Send uppercase message back to client, using ClientAddr as the address*/
    sendto(udpSocket,buffer,nBytes,0,(struct sockaddr *)&dest_Addr,dest_Addr_size);
	printf("MSG hase been sent to client...\n");
  }

  return 0;
}
