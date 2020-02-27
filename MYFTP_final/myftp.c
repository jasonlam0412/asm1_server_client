# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
# include <pthread.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#include "myftp.h"



P_message *list_request(){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	stpcpy(p_message->protocol, "myftp");
	p_message->type = 0xA1;
	p_message->length = 10;
	return p_message;
}

P_message *list_reply(char *filename, int filelength){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	stpcpy(p_message->protocol, "myftp");
	p_message->type = 0xA2;
	p_message->length = 0 + filelength;
	strcpy(p_message->payload,filename);
	return p_message;
}

P_message *get_request(char *filename, int filelength){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	stpcpy(p_message->protocol, "myftp");
	p_message->type = 0xB1;
	p_message->length = 0 + filelength;
	strcpy(p_message->payload,filename);
	return p_message;
}


P_message *get_reply(int exist){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	stpcpy(p_message->protocol, "myftp");
	if(exist){
		p_message->type = 0xB2;
	}else{
		p_message->type = 0xB3;
	}
	
	p_message->length = 0;
	return p_message;
}

P_message *put_request(char *filename, int filelength){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	stpcpy(p_message->protocol, "myftp");
	p_message->type = 0xC1;
	p_message->length = 0 + filelength;
	strcpy(p_message->payload,filename);
	return p_message;
}

P_message *put_reply(){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	stpcpy(p_message->protocol, "myftp");
	p_message->type = 0xC2;
	p_message->length = 0;
	return p_message;
}

P_message *file_data(){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	stpcpy(p_message->protocol, "myftp");
	p_message->type = 0xFF;
	p_message->length = 0;
	
	return p_message;
}

void print_debug(P_message *p_message){
	printf("protocol : %s\n",p_message->protocol);
	printf("type     : %x\n",p_message->type);
	printf("length   : %d\n",p_message->length);
	printf("payload  : %s\n",p_message->payload);
	printf("------------------------------------------\n");
}