#define TXTSIZE 128
#define MSGSIZE	1024
#define PACKETSIZE 512

#define TRANSMISSION_SUCCESS 1
#define TRANSMISSION_FAILURE 0

void Display(char*);
char* WRQmsg(char*);
int receiveAck(int,int);
void TransmitDatagram(int, char*, int);
