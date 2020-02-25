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

int main(int argc, char** argv){
	int len;
	char inputBuffer[256] = {};
	int sd=socket(AF_INET,SOCK_STREAM,0);
	int client_sd;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	server_addr.sin_port=htons(PORT);
	if(bind(sd,(struct sockaddr *) &server_addr,sizeof(server_addr))<0){
		printf("bind error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}
	if(listen(sd,3)<0){
		printf("listen error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}
	
	while(1){
		char fileName[] = "fuck.txt";
		int filelen = 9;
		P_message *LIST_REPLY = (P_message *) malloc(sizeof(P_message));
		int addr_len=sizeof(client_addr);
		if((client_sd=accept(sd,(struct sockaddr *) &client_addr,&addr_len))<0){
			printf("accept erro: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
		printf("recv type: %x", LIST_REPLY->type);
		//LIST_REPLY = list_reply(fileName, filelen);
		printf("send type: %x", LIST_REPLY->type);
		recv(client_sd,inputBuffer,sizeof(inputBuffer),0);
		
       printf("Get:%s\n",inputBuffer);
		
		
		if((len=recv(client_sd,LIST_REPLY,sizeof(*LIST_REPLY),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}else{
			//printf("recv type: %x\n", LIST_REPLY->type);
		}
		printf("recv type: %x\n", LIST_REPLY->type);
		/*printf("recv type: %x", LIST_REPLY->type);
		LIST_REPLY = list_reply(fileName, filelen);
		printf("send type: %x", LIST_REPLY->type);
		if((len=send(client_sd,(void*)LIST_REPLY,sizeof(*LIST_REPLY),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
		printf("send type: %x", LIST_REPLY->type);
		*/
		
		
	}
	close(sd);
	return 0;
}
