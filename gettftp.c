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
	
	char* res=malloc(lenfile+lenoct+4);
	
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

void ReceiveDatagram(int desc_sock, char* file){
	char * buf = malloc(PACKETSIZE);
	char * ack = malloc(4);
	
	struct sockaddr *rec = malloc(sizeof(struct sockaddr));
	socklen_t *reclen = malloc(sizeof (socklen_t));
	*reclen = sizeof(struct sockaddr);
	
	ssize_t readCount = 0;
	readCount = recvfrom(desc_sock,buf,PACKETSIZE,0,rec,reclen);
	
	
	if(readCount==-1){perror("Reading error");exit(EXIT_FAILURE);}

	
	int desc_file = open(file, O_RDWR);
	if(desc_file==-1){
		desc_file = open(file,O_RDWR | O_CREAT ,S_IRWXU | S_IRWXG | S_IRWXO); //if the requested file does not exist, we create it
	}
	
	//creation of the acknowledge message
	ack[0]=0;ack[1]=4;
	ack[2]=buf[2];ack[3]=buf[3]; 
	
	if(sendto(desc_sock,ack,4,0,rec,*reclen)==-1){perror("Sending error");exit(EXIT_FAILURE);};
	
	
	if(readCount==4){Display("End of Transmission !\n");exit(EXIT_SUCCESS);}
	
	if(ack[3]!=1 && ack[2]!=0){	//if the block is not the first of the transmission , we write at the end of the file
		lseek(desc_file,0,SEEK_END);
	}
	if(write(desc_file,&buf[4],readCount-4)<=0){Display("Writing error\n");exit(EXIT_FAILURE);}
	
	free(ack);free(buf);free(rec);free(reclen);
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
	
	while(1){
		ReceiveDatagram(desc_sock,file);
	}
}
