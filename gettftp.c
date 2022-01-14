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
	int lenfile = strlen(filename);
	int lenoct = strlen("octet");
	
	char* res=malloc(lenfile+strlen("octet")+4);
	
	//the two first bytes are "01"
	res[0]=0;
	res[1]=1;
	
	//The next byte are for the target file name
	strcpy(&res[2],filename);
	
	//The last bytes of the RRQ msg are for 0 "octet" 0
	res[2 + lenfile]=0;
	
	strcpy(&res[2 + 1 + lenfile],"octet");
	
	res[2 + 1 + lenfile + lenoct]=0;
	return res;
	}

void ReceiveData(int desc_sock, char* file){
	int desc_file;
	
	desc_file = open(file, O_RDWR);
	if(desc_file==-1){
		desc_file = open(file,O_RDWR | O_CREAT ,S_IRWXU | S_IRWXG | S_IRWXO); //if the requested file does not exist, we create it
	}
	dup2(desc_file,desc_sock);
	}

int main(int argc, char **argv){
	int systest;
	
	char* file = malloc(TXTSIZE);
	char* host = malloc(TXTSIZE);
	char* service = malloc(TXTSIZE);
	
	char* hostadress = malloc(TXTSIZE);
	char* serviceadress = malloc(TXTSIZE);
	
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_protocol = IPPROTO_UDP;
	
	service = NULL;
	
	if(argc<3){Display("Not enough arguments \n");exit(EXIT_FAILURE);}
	if(argc==3){host = argv[1];	file = argv[2];}
	if(argc==4){host = argv[1];	service = argv[2];file = argv[3];}
	
	int msgSize = 4 + strlen(file) + strlen("octet");
	char* msg = malloc(msgSize);
	
	systest = getaddrinfo(host,service,&hints,&res);
	if(systest!=0){Display((char*)gai_strerror(systest));exit(EXIT_FAILURE);}
	
	for (rp = res; rp != NULL; rp = rp->ai_next) {
		if(getnameinfo(rp->ai_addr,rp->ai_addrlen,hostadress,TXTSIZE,serviceadress,TXTSIZE,NI_NUMERICHOST | NI_NUMERICSERV)==0){
			Display(hostadress);
			Display("\n");
			Display(serviceadress);
			Display("\n");
			}
		}
	int desc_sock = socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	if(desc_sock==-1){Display("Socket failure \n");exit(EXIT_FAILURE);}
	
	msg = RRQmsg(file);
	sendto(desc_sock,msg,msgSize,0,res->ai_addr,res->ai_addrlen);
	free(msg);
	
}
