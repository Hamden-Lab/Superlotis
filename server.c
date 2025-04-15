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
    argc = 0;
    retval = 1;
    resplen=sprintf(response,"Invalid command.");

    cmd = strtok(buffer," =");
    // if the command is null, we're in trouble
    //   printf("Command [%d] %s\n",strlen(cmd),cmd);
    
    if(cmd != NULL){
      // see if there is an argument
      arg = strtok(NULL," ");
      printf("Command  %s\n",cmd);
      if(arg != NULL) printf("Argument %s\n",arg);
      
      if(arg!=NULL) argc=1;
      // exit command received
      if( strcmp(cmd,"exit")==0){
	retval = 0;
	resplen = sprintf(response,"Exit requested.");
      };

      // help
      if( strcmp(cmd,"help")==0){
	resplen = sprintf(response,"%s",helpstr);
      };

      // set/get exptime
      if (strcmp(cmd,"exptime")==0){
	if(argc == 1){
	  fval = atof(arg);
	    res = set_exposure_time(fval);
	    if(res){
	      resplen = sprintf(response,"Error setting exposure time.");
	    } else { 
	      resplen = sprintf(response,"%0.2f",fval);
	    };
	  } else {
	    res = get_exposure_time(&fval);
	    if (res){
	      resplen = sprintf(response,"Error getting exposure time.");
	    } else {
	      resplen = sprintf(response,"%0.2f",fval);
	    };
	}; //argument list
      };
    
      // set/get shutter mode
        if (strcmp(cmd,"shutter_mode")==0){
	if(argc == 1){
	  fval = atof(arg);
	    res = set_shutter(fval);
	    if(res){
	      resplen = sprintf(response,"Error setting shutter mode.");
	    } else { 
	      resplen = sprintf(response,"%0.2f",val);
	    };
	  } else {
	    res = get_shutter(&val);
	    if (res){
	      resplen = sprintf(response,"Error getting shutter mode.");
	    } else {
	      resplen = sprintf(response,"%0.2f",fval);
	    };
	}; //argument list
      };

      // set/get temp
        if (strcmp(cmd,"temp")==0){
	if(argc == 1){
	  fval = atof(arg);
	    res = set_temp(fval);
	    if(res){
	      resplen = sprintf(response,"Error setting temp.");
	    } else { 
	      resplen = sprintf(response,"%0.2f",val);
	    };
	  } else {
	    res = get_temp(&fval);
	    if (res){
	      resplen = sprintf(response,"Error getting temp.");
	    } else {
	      resplen = sprintf(response,"%0.2f",fval);
	    };
	}; //argument list
      };

      // set/get analog gain
        if (strcmp(cmd,"analog_gain")==0){
	if(argc == 1){
	  fval = atof(arg);
	    res = set_analog_gain(fval);
	    if(res){
	      resplen = sprintf(response,"Error setting analog gain.");
	    } else { 
	      resplen = sprintf(response,"%0.2f",val);
	    };
	  } else {
	    res = get_analog_gain(&val);
	    if (res){
	      resplen = sprintf(response,"Error getting analog gain.");
	    } else {
	      resplen = sprintf(response,"%0.2f",val);
	    };
	}; //argument list
      };

//new
  //       if (strcmp(cmd,"rois")==0){
	// if(argc == 4){
	//   fval = atof(arg);
	//     res = set_rois(fval);
	//     if(res){
	//       resplen = sprintf(response,"Error setting roi values.");
	//     } else { 
	//       resplen = sprintf(response,"%0.2f",val);
	//     };
	//   } else {
	//     res = get_analog_gain(&val);
	//     if (res){
	//       resplen = sprintf(response,"Error getting analog gain.");
	//     } else {
	//       resplen = sprintf(response,"%0.2f",val);
	//     };
	// }; //argument list
  //     };

      // burst
    if (strcmp(cmd, "burst") == 0) {
        if (argc == 1) {  
            fval = atof(arg);  // Ensure fval is declared here
            res = burst((int)fval);
            if (res == 0) {
                resplen = sprintf(response, "Burst exposure of %d images complete.", (int)fval);
            } else {
                resplen = sprintf(response, "Burst exposure failed.");
            }
        } else {
            resplen = sprintf(response, "Usage: burst <number_of_exposures>");
        }
    }
//end new
     if (strcmp(cmd,"expose")==0){
	res = expose("exposure_file.raw");
	if (res){
	  resplen = sprintf(response,"Exposure error.");
	} else {
	  resplen = sprintf(response,"Exposure complete.");
	};
      };

      if (strcmp(cmd,"dark")==0){
	res = dark("dark_file.raw");
	if (res){
	  resplen = sprintf(response,"Exposure error.");
	} else {
	  resplen = sprintf(response,"Exposure complete.");
	};
      };

      if (strcmp(cmd,"bias")==0){
	res = bias("bias_file.raw");
	if (res){
	  resplen = sprintf(response,"Exposure error.");
	} else {
	  resplen = sprintf(response,"Exposure complete.");
	};
      };

      
    } else {
      // if the command is null, return the defaults
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
