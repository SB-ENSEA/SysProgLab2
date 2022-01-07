#include "puttftp.h"
#include <sys/types.h>
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

void Display(char * text){
	write(STDOUT_FILENO,text,strlen(text));
	}

int main(int argc, char **argv){
	char* file;
	char* host;
	if(argc<3){Display("Not enough arguments \n");exit(EXIT_FAILURE);}
	host = argv[1];
	file = argv[2];
	
	}
