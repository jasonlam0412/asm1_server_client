# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
# include <dirent.h>
# include <pthread.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#include "myftp.h"
# define PORT 12345
int list_compare(char *fileRequest);
void send_list(int client_sd);
void client_action(int client_sd);



int main(int argc, char** argv){
	int len;
	char inputBuffer[256] = {};
	int sd=socket(AF_INET,SOCK_STREAM,0);
    long val = 1;
    if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(long)) == -1){
        perror("setsockopt");
        exit(1);
    }
	int client_sd;
	struct sockaddr_in server_addr;
	//struct sockaddr_in client_addr;
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
		struct sockaddr_in client_addr;
		
		int addr_len=sizeof(client_addr);
		if((client_sd=accept(sd,(struct sockaddr *) &client_addr,&addr_len))<0){
				printf("accept erro: %s (Errno:%d)\n",strerror(errno),errno);
				exit(0);
			}
		
        client_action(client_sd);
		
		
		
	}
	close(sd);
	return 0;
}


int list_compare(char *fileRequest){
	DIR *folder;
	struct dirent *entry;
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));
	strcat(cwd, "/data");
	folder = opendir(cwd);
	//char *filename = NULL;
	int bingo = 0;
	while((entry = readdir(folder)) != NULL){
		if(strcmp(entry->d_name, fileRequest) == 0){
			bingo =1;
			break;
		}
	}
	
	return bingo;
	
}

void send_list(int client_sd){
	int len;
	P_message *REPLY = (P_message *) malloc(sizeof(P_message));
	DIR *folder;
	struct dirent *entry;
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));
	strcat(cwd, "/data");
	folder = opendir(cwd);
	
	char *filename = NULL;
		
	while((entry = readdir(folder)) != NULL){
		filename = (char *) realloc(filename, sizeof(filename) + sizeof(entry->d_name) + 1);
		strcat(filename, entry->d_name);
		strcat(filename, "\n");
	}
	int size;
	size = strlen(filename);
	size = htonl(size);
	
	REPLY = list_reply(filename, size);
	REPLY->length = htonl(REPLY->length);
	if((len=send(client_sd,REPLY,sizeof(*REPLY),0))<0){
		printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}
	free(REPLY);
}

void client_action(int client_sd){
	char filename[] = "fuck.txt";
	int filelen = 9;
	int len;
	P_message *REPLY = (P_message *) malloc(sizeof(P_message));
	P_message *REQUEST = (P_message *) malloc(sizeof(P_message));
	//LIST_REPLY = list_reply(fileName, filelen);
	//recv(client_sd,inputBuffer,sizeof(inputBuffer),0);
	
	//printf("Get:%s\n",inputBuffer);
	
	
	if((len=recv(client_sd,REQUEST,sizeof(*REQUEST),0))<0){
		printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}
	
	if(REQUEST->type == 0xA1){
		printf("stype LIST    : %x\n",REQUEST->type);
		send_list(client_sd);
	}else if(REQUEST->type == 0xB1){
		printf("stype GET    : %x\n",REQUEST->type);
		if(list_compare(REQUEST->payload)){
			printf("File found\n");
		}else{
			printf("No such file\n");
		}
		REPLY = list_reply(filename, filelen);
		//printf("send type: %x", LIST_REPLY->type);
		if((len=send(client_sd,REPLY,sizeof(*REPLY),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
	}else if(REQUEST->type == 0xC1){
		printf("stype PUT    : %x\n",REQUEST->type);
		REPLY = list_reply(filename, filelen);
		//printf("send type: %x", LIST_REPLY->type);
		if((len=send(client_sd,REPLY,sizeof(*REPLY),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
	}else{
		printf("Fail\n");
	}

	print_debug(REQUEST);
	free(REQUEST);
	free(REPLY);
	
}