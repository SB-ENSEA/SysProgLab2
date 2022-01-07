#include "gettftp.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>

struct addrinfo hints;
struct addrinfo *res,*rp;

void Display(char * text){
	write(STDOUT_FILENO,text,strlen(text));
	}


int main(int argc, char **argv){
	int systest;
	
	char* file = malloc(TXTSIZE);
	char* host = malloc(TXTSIZE);
	char* service = malloc(TXTSIZE);
	
	char* hostadress = malloc(TXTSIZE);
	char* servadress = malloc(TXTSIZE);
	
	memset(&hints,0,sizeof(struct addrinfo));
	
	service = NULL;
	
	if(argc<3){Display("Not enough arguments \n");exit(EXIT_FAILURE);}
	if(argc==3){host = argv[1];	file = argv[2];}
	if(argc==4){host = argv[1];	service = argv[2];file = argv[3];}
	
	
	systest = getaddrinfo(host,service,&hints,&res);
	if(systest!=0){Display((char*)gai_strerror(systest));exit(EXIT_FAILURE);}
	
	for (rp = res; rp != NULL; rp = rp->ai_next) {
		if(getnameinfo(rp->ai_addr,rp->ai_addrlen,hostadress,TXTSIZE,servadress,TXTSIZE,NI_NUMERICHOST | NI_NUMERICSERV)==0){
			Display(hostadress);
			Display("\n");
			Display(servadress);
			Display("\n");
			break;
			}
		}
	int sockdesc = socket(PF_INET,SOCK_DGRAM,0);
	if(sockdesc==-1){Display("Socket failure \n");exit(EXIT_FAILURE);}
	
	if(connect(sockdesc,res->ai_addr,res->ai_addrlen)==-1){Display("Connection failure\n");}
	else{Display("Connection Established !\n");}
	
}
