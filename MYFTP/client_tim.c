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
	
	// LIST-------------------------------------------------------
	if(strcmp(argv[3],"list")==0){
		
        P_message *LIST_REQUEST = list_request();
        
        // If there is a error in sending, exit
		if((len=send(sd,LIST_REQUEST,sizeof(*LIST_REQUEST),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
		// recive required filenames
		if((len=recv(sd,LIST_REQUEST,sizeof(*LIST_REQUEST),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
		// LIST-------------------------------------------------END
		// GET------------------------------------------------------
	}else if(strcmp(argv[3],"get")==0){
		P_message *GET_REQUEST = get_request(argv[4], strlen(argv[4]));
        
        //If error occurs, exit
		if((len=send(sd,GET_REQUEST,sizeof(*GET_REQUEST),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
		
		//Open the file for writing
		FILE* fp;
        if ( (fp = fopen(argv[4], "wb")) == NULL){
            perror("fopen");
            exit(1);
        }
        //Receive the content of file
        recv(sd,GET_REQUEST,sizeof(*GET_REQUEST),0);
        while(GET_REQUEST->type == 0xB2){
            char buf[100];
            recv(sd,GET_REQUEST,sizeof(*GET_REQUEST),0);
            if(GET_REQUEST->type == 0xB3){
                break;
            }
            recv(sd, buf, sizeof(buf), 0);
            fprintf(fp, "%s", buf);
        }
        fclose(fp);
        //GET-----------------------------------------------END
        //PUT--------------------------------------------------
	}else if(strcmp(argv[3],"put")==0){
        FILE* fp;
        if ( (fp = fopen(argv[4], "rb")) == NULL){
            perror("fopen");
            exit(1);
        }
        P_message *PUTREQUEST = put_request(argv[4], strlen(argv[4]));
        
        //PUT------------------------------------------------END
	}else{
		printf("Only input list/get/put\n");
		exit(0);
	}
	
	close(sd);
	return 0;
}

