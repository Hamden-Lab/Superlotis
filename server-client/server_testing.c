#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
// #include "picam.h"

#include "camera.h"
#include "server.h"
#include "socketid.h"

const char helpstr[]=" \
\nCommands:\
\nanalog_gain [piint]\
\ntemp [piflt]\
\nshutter_mode [piint]\
\nexptime [piflt]\
\nexpose\
\ndark\
\nbias\
\nstatus\
\nburst\
\ncommit_params\n";


void error(const char *msg)
{
    perror(msg);
}

// variables needed for sockets
int sockfd, newsockfd, portno;
socklen_t clilen;
char buffer[256], response[2048];
int resplen, n;
int port_open = 0;
struct sockaddr_in serv_addr, cli_addr;
int argct, retval;


int open_server(){
  // make a socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    error("ERROR opening socket");
    return -1;
  };

  // set options and clear values
  struct linger lo = { 1, 0 };
  setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &lo, sizeof(lo));
  bzero((char *) &serv_addr, sizeof(serv_addr));

  portno = CAM_PORT;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
	   sizeof(serv_addr)) < 0){
    error("ERROR on binding");
    return -1;
  };
  
  // set up listening.
  listen(sockfd,5);
  clilen = sizeof(cli_addr);

  port_open = 1;
  printf("Server opened.\n");
  return 0;
};

int close_server(){
  if(port_open){
    close(sockfd);
    printf("Server closed\n");
    port_open = 0;
  }; // port_open
  return 0;
};

int listen_server(){
  char *cmd;
  char *arg;
  int argc;
  int res,val;
  piflt fval;
  if(port_open){
    newsockfd = accept(sockfd, 
		       (struct sockaddr *) &cli_addr, 
		       &clilen);
    if (newsockfd < 0) {
      error("ERROR on accept");
      return 0;
    };
    bzero(buffer,256);
    n = read(newsockfd,buffer,255);
    if (n < 0){
      error("ERROR reading from socket");
      return 0;
    };
    printf("Received command: %s.\n",buffer);
    printf("%d\n",strcmp(buffer,"exit"));

    // *********************************************
    // OK. Here is where we process commands
    // set up the defaults;

    // argc = 0;
    retval = 1;
    resplen=sprintf(response,"Invalid command.");
    cmd = strtok(buffer,"=");
    arg = strtok(NULL,"=");

    if(arg==NULL){

    }
    
    if(cmd != NULL){
      printf("Command  %s\n",cmd);
      // if(arg!=NULL) argc=0;
      if (strcmp(cmd,"exptime")==0){
        if (argc == 0){
          printf("Getter");
        }else{
          printf("Setter");
          printf(arg);
          fval = atof(arg);
          printf("Value: %f", fval);
          // res = get_exposure_time(&fval);
          // if (res){
          //   resplen = sprintf(response,"Error getting exposure time.");
          //   } else {
          //   resplen = sprintf(response,"%0.2f",&fval);
          //   };
        };

      };

    };
    
    
    // *********************************************
    
    n = write(newsockfd,response,resplen);
    if (n < 0) error("ERROR writing to socket");
    close(newsockfd);
    return retval;
  } else { 
    return 0;
  };
  return 1;
};

    //1. check to see if there is an equal sign
    //2. if there is an equal sign, cmd = strtok(buffer, "="), and arg = strtok(None,"=")
    //3. if there is no equal sign, cmd = buffer, arg = 0

    //if there is a "=" sign, then there cmd = strtok(buffer, "=") 
    //(i.e. cmd is the first of the input (buffer))
    //---> as a result, we get arg = strtok(None,"=") (strtok picks up where cmd left off, arg is all values after "=")
    //---> ---> if arg!=NULL, argc = 1
    // if there is no 


    // if(cmd != NULL){
    //   // see if there is an argument
    //   arg = strtok(NULL,"=");
    //   printf("Command  %s\n",cmd);
    //   if(arg != NULL) 