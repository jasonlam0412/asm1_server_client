# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
# include <pthread.h>
# include <dirent.h>
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
		//char message[] = {"Hi there"};
		//send(sd,message,sizeof(message),0);
        
        print_debug(LIST_REQUEST);
        
		if((len=send(sd,LIST_REQUEST,sizeof(*LIST_REQUEST),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
		
		if((len=recv(sd,LIST_REQUEST,sizeof(*LIST_REQUEST),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
		
		print_debug(LIST_REQUEST);
		
	}else if(strcmp(argv[3],"get")==0){
		char file_request[50];
		if(argc > 4){
			strcpy(file_request, argv[4]);
			printf("%s\n", file_request);
			printf("%d\n", strlen(file_request));
		}else{
			printf("Please enter file name\n");
		}
		
		P_message *GET_REQUEST = get_request(file_request, strlen(file_request));	
		print_debug(GET_REQUEST);
		if((len=send(sd,GET_REQUEST,sizeof(*GET_REQUEST),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
		
		if((len=recv(sd,GET_REQUEST,sizeof(*GET_REQUEST),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
		
		
        print_debug(GET_REQUEST);
	
	}else if(strcmp(argv[3],"put")==0){
		char file_put[50];
		if(argc > 4){
			strcpy(file_put, argv[4]);
			printf("%s\n", file_put);
			printf("%d\n", strlen(file_put));
		}else{
			printf("Please enter file name\n");
		}
		
		P_message *PUT_REQUEST = put_request(file_put, strlen(file_put));	
		
		print_debug(PUT_REQUEST);
		
		if((len=send(sd,PUT_REQUEST,sizeof(*PUT_REQUEST),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
		
		if((len=recv(sd,PUT_REQUEST,sizeof(*PUT_REQUEST),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
		
		
        print_debug(PUT_REQUEST);
		
	}else{
		printf("Only input list/get/put\n");
		exit(0);
	}
	
	close(sd);
	return 0;
}