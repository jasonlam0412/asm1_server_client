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
# define PORT 12345
int portNum, sd;
char Ip[15];
int main(int argc, char** argv){
	int len;
	
	if(argc < 3 || argc > 5){
		printf("Command error\n");
		exit(0);
	}
	portNum = atoi(argv[2]);
	sd=socket(AF_INET,SOCK_STREAM,0);
	strcpy(Ip,argv[1]);
	struct sockaddr_in server_addr;
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	server_addr.sin_port=htons(PORT);
	
	if(connect(sd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
		printf("connection error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}
	//int le=0x12345678;
	//printf("%x\n",le);
	//le=htonl(le);
	//printf("%x\n",le);
	
	if(strcmp(argv[3],"list")==0){
		P_message *LIST_REQUEST = list_request();
		char message[] = {"Hi there"};
		send(sd,message,sizeof(message),0);
        
        printf("protocol : %s\n",LIST_REQUEST->protocol);
        printf("type     : %u\n",LIST_REQUEST->type);
        printf("length   : %d\n",LIST_REQUEST->length);
        printf("payload  : %s\n",LIST_REQUEST->payload);
        printf("------------------------------------------\n");
        
		if((len=send(sd,LIST_REQUEST,sizeof(*LIST_REQUEST),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}else{
			printf("protocol:%s\n", LIST_REQUEST->protocol);
			printf("type %x\n", LIST_REQUEST->type);
		}
		
		if((len=recv(sd,LIST_REQUEST,sizeof(*LIST_REQUEST),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
		
		printf("protocol : %s\n",LIST_REQUEST->protocol);
        printf("type     : %u\n",LIST_REQUEST->type);
        printf("length   : %d\n",LIST_REQUEST->length);
        printf("payload  : %s\n",LIST_REQUEST->payload);
	}else if(strcmp(argv[3],"get")==0){
		
	}else if(strcmp(argv[3],"put")==0){
		
	}else{
		printf("Only input list/get/put\n");
		exit(0);
	}
	
	close(sd);
	return 0;
}

