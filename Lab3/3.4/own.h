#include <sys/socket.h>
#include <netinet/in.h>

#ifndef OWN_H_   /* Include guard */
#define OWN_H_


#define MSG_ERR "-ERR\r\n"
#define MSG_OK  "+OK\r\n"
#define MSG_QUIT  "QUIT"
#define MSG_GET "GET"

#define MAX_STR 1023

#define MAXBUFL 1024

#define   TCP  0
#define  UDP   1

extern int connectin_connected;

struct arg_struct {
    int socket;
    int connectin_connected;
};


int find_patterm(char source[1024], char pattern[1024]);  /*  */
void  config( struct   sockaddr_in * serverAddr, char ip[1024] , char port[1024]  );
int Socket(int trans  );  //  Zero for TCP 1 for UPD
void  get_ip(char ipadrs[1024],  struct sockaddr_in * clientAddr);
int  get_port( struct sockaddr_in * clientAddr );
int  send_file();
int  rec_file();
int rec_command( int sd );
int send_command_rec_file( int sd ,  char cmd[1024] ,   char filename[1024] );
int send_command_rec_file_interactive( int sd ,  char cmd[1024] ,   char filename[1024] ) ;
void check_arg( int narg , int rnarg  );





#endif // OWN_H_
