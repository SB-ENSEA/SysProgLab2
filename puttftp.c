#include "puttftp.h"
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

char* WRQmsg(char* filename){
	int lenfile = strlen(filename);
	int lenoct = strlen("octet");
	
	char* res=malloc(lenfile+lenoct+4);
	
	//the two first bytes are "02"
	res[0]=0;
	res[1]=2;
	
	//The next byte are for the target file name
	strcpy(&res[2],filename);
	
	//The last bytes of the RRQ msg are for 0 "octet" 0
	res[2 + lenfile]=0;
	
	strcpy(&res[2 + 1 + lenfile],"octet");
	
	res[2 + 1 + lenfile + lenoct]=0;
	return res;
	}

void ErrHandler(char* ErrPacket){
	
	if(ErrPacket[2] == 0 && ErrPacket[3] == 0){
			Display(&ErrPacket[4]);
			Display("\n");
			exit(EXIT_FAILURE);
		}
	else{
			char* disp = malloc (MSGSIZE);
			sprintf(disp,"Error code : %d \n",ErrPacket[3]);
			Display(disp);
			exit(EXIT_FAILURE);
		}
	}

int receiveAck(int desc_sock,int BlockNb){
	
	struct sockaddr *rec = malloc(sizeof(struct sockaddr));
	socklen_t *reclen = malloc(sizeof (socklen_t));
	*reclen = sizeof(struct sockaddr);
	
	char * ack = malloc(4);
	ssize_t readCount = 0;
	
	readCount = recvfrom(desc_sock,ack,4,0,rec,reclen);
	if(readCount==-1){perror("Reading error");exit(EXIT_FAILURE);}
	
	if (ack[1]==5){ErrHandler(ack);} //In case of an error packet, we terminate connection
	
	if (ack[1]!=4){return TRANSMISSION_FAILURE;} //if the packet is not an acknoledgment or an error, we send our datagam again

	if(BlockNb/10 == ack[2] && BlockNb%10 == ack[3]){//ack[1]=4 => acknowledge packet,ack[2] and ack[3] check if the block number is the right one 
		return TRANSMISSION_SUCCESS;
		}
	return TRANSMISSION_FAILURE;
	}

void TransmitDatagram(int desc_sock, char* file, int BlockNb){
	char * buf = malloc(PACKETSIZE);
	
	struct sockaddr *rec = malloc(sizeof(struct sockaddr));
	socklen_t *reclen = malloc(sizeof (socklen_t));
	*reclen = sizeof(struct sockaddr);
	
	ssize_t readCount = 0;
	
	int desc_file = open(file, O_RDWR);
	if(desc_file==-1){perror("Can't open file");}
	
	//if the block is not the first of the transmission , we must move the cursor at the right place
	if(BlockNb!=1){	
		lseek(desc_file,512 * BlockNb,SEEK_SET); // the case where [512 * BlockNb < size of the file] is caught at the previous iteration with the readCount < 512 condition.
	}
	
	//creation of the Datagram header
	buf[0]=0;buf[1]=3;
	buf[2]=BlockNb/10;buf[3]=BlockNb%10; 
	
	readCount = read(desc_file,&buf[4],512);
	if(readCount==-1){perror("Can't read file");exit(EXIT_FAILURE);}
	
	if(sendto(desc_sock,buf,4 + 512,0,rec,*reclen)==-1){perror("Sending error");exit(EXIT_FAILURE);};
	
	if(readCount < 512){// we have read the file up to the end, thus we end the transmission
		receiveAck(desc_sock,BlockNb);
		sendto(desc_sock,buf,6,0,rec,*reclen);//to end the transmission, we just have to transmit a datagram with less than 512 bytes of data
		free(buf);free(rec);free(reclen);
		}
	free(buf);free(rec);free(reclen);
}

int main(int argc, char **argv){
	int systest;
	
	char* file = malloc(TXTSIZE);
	char* host = malloc(TXTSIZE);
	char* service = malloc(TXTSIZE);
	
	char* hostadress = malloc(TXTSIZE);
	char* serviceadress = malloc(TXTSIZE);
	
	int BlockNb = 0;
	
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_protocol = IPPROTO_UDP;
	
	service = NULL;
	
	if(argc<3){Display("Not enough arguments \n");exit(EXIT_FAILURE);}
	if(argc==3){host = argv[1];	file = argv[2];}
	if(argc==4){host = argv[1];	service = argv[2];file = argv[3];}
	
	int msgSize = 4 + strlen(file) + strlen("octet");
	
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

	while(1){
		if(BlockNb == 0){sendto(desc_sock,WRQmsg(file),msgSize,0,res->ai_addr,res->ai_addrlen);}// we consider datagram 0 to be the write RQ message
		else{TransmitDatagram(desc_sock,file,BlockNb);}
		if(receiveAck(desc_sock,BlockNb) == TRANSMISSION_SUCCESS){BlockNb++;} //if we receive the right acknowledgement packet, we can transmit the next datagram; else we send again the same datagram
	}
}
