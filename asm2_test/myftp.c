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
	memset(p_message->payload, 0, 500);
	strcpy(p_message->protocol, "myftp");
	p_message->type = 0xA1;
	p_message->length = 0;
	return p_message;
}

P_message *list_reply(char *filename, int filelength){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	memset(p_message->payload, 0, 500);
	strcpy(p_message->protocol, "myftp");
	p_message->type = 0xA2;
	p_message->length = 0 + filelength;
	strcpy(p_message->payload,filename);
	//memset(p_message->payload, 0, 500);
	return p_message;
}

P_message *get_request(char *filename, int filelength){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	memset(p_message->payload, 0, 500);
	strcpy(p_message->protocol, "myftp");
	p_message->type = 0xB1;
	p_message->length = 0 + filelength;
	strcpy(p_message->payload,filename);
	return p_message;
}


P_message *get_reply(int exist){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	memset(p_message->payload, 0, 500);
	strcpy(p_message->protocol, "myftp");
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
	memset(p_message->payload, 0, 500);
	strcpy(p_message->protocol, "myftp");
	p_message->type = 0xC1;
	p_message->length = 0 + filelength;
	strcpy(p_message->payload,filename);
	return p_message;
}

P_message *put_reply(){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	memset(p_message->payload, 0, 500);
	strcpy(p_message->protocol, "myftp");
	p_message->type = 0xC2;
	p_message->length = 0;
	return p_message;
}

P_message *file_data(){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	memset(p_message->payload, 0, 500);
	strcpy(p_message->protocol, "myftp");
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

void sendP_message(P_message *send_message, int sd){
	int len;
	send_message->length = htonl(send_message->length);
	//send_message->payload = htonl(send_message->payload);
	if((len=send(sd,send_message,sizeof(*send_message),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
}

P_message *recvP_message(int sd){
	P_message *recv_message = (P_message*)malloc(sizeof(P_message));
	int len;
	if((len=recv(sd,recv_message,sizeof(*recv_message),0))<0){
			printf("Receive Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
	recv_message->length = ntohl(recv_message->length);
	//recv_message->payload = ntohl(recv_message->payload);
	return recv_message;
}
Stripe *stripe_init(int n, int k, int block_size, int no_of_block){
	int i;
	Stripe *stripe = malloc(sizeof(Stripe));
    stripe->encode_matrix = malloc(sizeof(unsigned char) * (n * k));
	//stripe->decode_matrix = malloc(sizeof(unsigned char) * (n * k));
	stripe->invert_matrix = malloc(sizeof(unsigned char) * (k * k));
	stripe->error_matrix = malloc(sizeof(unsigned char) * (k * k));
    stripe->table = malloc(sizeof(unsigned char) * (32 * k * (n-k)));
    stripe->blocks = malloc(sizeof(unsigned char **) * no_of_block);
    printf("%d\n",(int)sizeof(stripe->blocks));
    for (i = 0; i < no_of_block; i++){
    	 printf("GOOD 2\n");
    	stripe->blocks[i] = malloc(sizeof(unsigned char *) * block_size);
		memset(stripe->blocks[i],0,block_size);
    }
	return stripe;
}

