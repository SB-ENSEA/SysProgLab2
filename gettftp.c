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

char* RRQmsg(char* filename){
	char* res=malloc((strlen(filename)+strlen("octet")+4)*sizeof(char));
	res[0]=0;
	res[1]=1;
	strcpy(&res[2],filename);
	res[2+strlen(filename)] = 0;
	strcpy(&res[3+strlen(filename)],"octet");
	res[3+strlen("octet")+strlen(filename)] = 0;
	return res;
	}

int main(int argc, char **argv){
	int systest;
	
	char* file = malloc(TXTSIZE);
	char* host = malloc(TXTSIZE);
	char* service = malloc(TXTSIZE);
	
	char* hostadress = malloc(TXTSIZE);
	char* servadress = malloc(TXTSIZE);
	
	char* msg = malloc (MSGSIZE);
	
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
	int sockdesc = socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	if(sockdesc==-1){Display("Socket failure \n");exit(EXIT_FAILURE);}
	
	msg = RRQmsg(file);
	Display(msg);
	sendto(sockdesc,msg,sizeof(msg),0,res,res->ai_addrlen);
}
