#define TXTSIZE 128
#define MSGSIZE	1024
#define PACKETSIZE 512

void Display(char*);
char* RRQmsg(char*);
void ReceiveDatagram(int,char*);
