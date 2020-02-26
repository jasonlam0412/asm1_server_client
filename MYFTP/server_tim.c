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
# include <limits.h>
# include "myftp.h"
# define PORT 12345

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
    int addr_len = sizeof(client_addr);
	
	
	while(1){
        if((client_sd=accept(sd,(struct sockaddr *) &client_addr,&addr_len))<0){
            printf("accept erro: %s (Errno:%d)\n",strerror(errno),errno);
            exit(0);
        }else {
            printf("receive connection from %s\n", inet_ntoa(client_addr.sin_addr));
        }
		
		P_message *RESPONSE = (P_message *) malloc(sizeof(P_message));
		int addr_len=sizeof(client_addr);
		
		if((len=recv(client_sd,RESPONSE,sizeof(*RESPONSE),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}else{
			//printf("recv type: %x\n", LIST_REPLY->type);
		}
		printf("protocol : %s\n",RESPONSE->protocol);
        printf("type     : %u\n",RESPONSE->type);
        printf("length   : %d\n",RESPONSE->length);
        printf("payload  : %s\n",RESPONSE->payload);
        
        // LIST-------------------------------------------------------
        if(RESPONSE->type == 0xA1){
            char* fileName = get_filenames();
            RESPONSE = list_reply(fileName, strlen(fileName));
            printf("send type: %x\n", RESPONSE->type);
            
            if((len=send(client_sd,RESPONSE,sizeof(*RESPONSE),0))<0){
                printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
                exit(0);
            }
            free(RESPONSE);
        }
        // LIST------------------------------------------------------END
        
        // GET----------------------------------------------------------
        if(RESPONSE->type == 0xB1){
            printf("RESPONSE->type == 0xB1\n");
            FILE* fp;
            char file_name[60];
            strcpy(file_name, "./data/");
            strcat(file_name, RESPONSE->payload);
            if ( (fp = fopen(file_name, "rb")) == NULL){
                perror("fopen");
                exit(1);
            } 
            RESPONSE = get_reply(1);
            len=send(client_sd,RESPONSE,sizeof(*RESPONSE),0);
            char buf[100];
            while(fgets(buf,100,fp) != NULL){
                send(client_sd,RESPONSE,sizeof(*RESPONSE),0);
                send(client_sd, buf, sizeof(buf), 0);
            }
            RESPONSE = get_reply(0);
            send(client_sd,RESPONSE,sizeof(*RESPONSE),0);
            fclose(fp);
            free(RESPONSE);
        }
        // GET-------------------------------------------------------END
		
	}
	close(sd);
	return 0;
}
