# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
# include <dirent.h>
# include <pthread.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/stat.h> 
# include <netinet/in.h>
# include <arpa/inet.h>
#include "myftp.h"
# define PORT 12345
#define MAX 1000
int list_compare(char *fileRequest);
void send_list(int client_sd);
void *client_action(void *client_sd);
void send_toClient(int client_sd, char *filename);
void recv_fromClient(int sd);
void file_read(char** argv);
int n, k, ID, block_size, port_number;
int no_of_block;
pthread_t *thread_id;
int thread_num = 0;

void file_read(char** argv){
	FILE *fp;
	char *line = NULL;
    size_t length = 0;
    ssize_t read;

    fp = fopen("serverconfig.txt", "r");
    if (fp == NULL){
    	printf("File does not exist.\n");
    	exit(0);
    }
    printf("Read file Success\n");

    //int n, k, ID, block_size, port_number;

    int o = 0;
    int temp_input[5];

    while((read = getline(&line, &length, fp)) != -1){
    	int temp = atoi(line);
    	temp_input[o] = temp;
    	o++;
    }
    n = temp_input[0];
    k = temp_input[1];
    ID = temp_input[2];
    block_size = temp_input[3];
    port_number = temp_input[4];
    printf("n = %d\n",n);
    printf("k = %d\n",k);
    printf("ID = %d\n",ID);
    printf("block_size = %d\n",block_size);
    printf("port_number = %d\n",port_number);
    fclose(fp);
}


int main(int argc, char** argv){
	int len;
	char inputBuffer[256] = {};
	thread_id = (pthread_t *) malloc(sizeof(pthread_t));
	int sd=socket(AF_INET,SOCK_STREAM,0);
    long val = 1;
	file_read(argv);
	if(argc != 2){
		printf("Fuck input port number\n");
		exit(1);
	}
	int port_num = atoi(argv[1]);
    if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(long)) == -1){
        perror("setsockopt");
        exit(1);
    }else{
		printf("setsockopt Sucsess\n");
	}
	int client_sd;
	struct sockaddr_in server_addr;
	//struct sockaddr_in client_addr;
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	server_addr.sin_port=htons(port_num);
	if(bind(sd,(struct sockaddr *) &server_addr,sizeof(server_addr))<0){
		printf("bind error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}else{
		printf("Binding     \n");
	}
	if(listen(sd,6)<0){
		printf("listen error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}else{
		printf("Listening\n");
	}
	
	while(1){
		struct sockaddr_in client_addr;
		
		int addr_len=sizeof(client_addr);
		if((client_sd=accept(sd,(struct sockaddr *) &client_addr,&addr_len))<0){
				printf("accept erro: %s (Errno:%d)\n",strerror(errno),errno);
				exit(0);
			}
		printf("accept SUCCESS");
		pthread_create(&thread_id[thread_num], NULL, client_action, &client_sd);
		thread_num += 1;
		pthread_t *newPointer = (pthread_t *) realloc(thread_id, sizeof(pthread_t) * (thread_num + 1));
		if(newPointer != NULL){
			thread_id = newPointer;
		}
		for(int i=0; i<thread_num; i++){
			pthread_join(thread_id[i], NULL);
		}

        
		
		
		
	}
	close(client_sd);
	close(sd);
	return 0;
}


void send_toClient(int client_sd, char *file){
	char filename[60];
	filename[0] ='\0';
	int len;
	strcat(filename,"data/");
	strcat(filename,file);
	printf("%s", filename);
	FILE *fp = fopen(filename,"rb");
	struct stat *filestat = (struct stat *)malloc(sizeof(struct stat));
	P_message *FILE_DATA = (P_message *) malloc(sizeof(P_message));
	FILE_DATA = file_data();
	char buf[1024];
	int numbytes;
	if(fp == NULL){
		printf("ERROR in open file\n\n");
		return;
	}
	if ( lstat(filename, filestat) < 0){
		exit(1);
	}
	printf("The file size is %lun\n", filestat->st_size);
	fseek(fp, 0, SEEK_END); 	
	int file_size = ftell(fp); 	
	fseek(fp, 0, SEEK_SET); 
	FILE_DATA->length = file_size;
	FILE_DATA->length = FILE_DATA->length;
	strcpy(FILE_DATA->payload,file);
	/*if((len=send(client_sd,FILE_DATA,sizeof(*FILE_DATA),0))<0){
		printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}*/
	sendP_message(FILE_DATA,client_sd);
	
	while(!feof(fp)){
		numbytes = fread(buf, sizeof(char), sizeof(buf), fp);
		printf("fread %d bytes, ", numbytes);
		numbytes = write(client_sd, buf, numbytes);
		printf("Sending %d bytesn",numbytes);
		
	}
	printf("\n");
	free(FILE_DATA);
	free(filestat);
	fclose(fp);
	
}

void recv_fromClient(int sd){
	P_message *FILE_DATA = (P_message *) malloc(sizeof(P_message));
	FILE_DATA = file_data();
	int len;
	/*if((len=recv(sd,FILE_DATA,sizeof(*FILE_DATA),0))<0){
		printf("recv Error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}*/
	FILE_DATA = recvP_message(sd);
	char filename[60];
	filename[0] ='\0';
	strcat(filename,"data/");
	strcat(filename,FILE_DATA->payload);
	print_debug(FILE_DATA);
	FILE *fp = fopen(filename, "wb");
	int numbytes;
	char buf[1024];
	print_debug(FILE_DATA);
	int fileSize = FILE_DATA->length;
	do{
		numbytes = read(sd, buf, sizeof(buf));
		printf("%d\n", sizeof(buf));
		printf("read %d bytes, ", numbytes);
		if(numbytes == 0){
			break;
		}
		numbytes = fwrite(buf, sizeof(char), numbytes, fp);
		printf("fwrite %d bytesn", numbytes);
		fileSize -= sizeof(buf);
	}while(1);
	free(FILE_DATA);
	fclose(fp);
}

int list_compare(char *fileRequest){
	DIR *folder;
	struct dirent *entry;
	char cwd[MAX];
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
	char cwd[MAX];
	getcwd(cwd, sizeof(cwd));
	strcat(cwd, "/data");
	folder = opendir(cwd);
	
	char *filename = NULL;
		
	while((entry = readdir(folder)) != NULL){
		filename = (char *) realloc(filename, sizeof(filename) + sizeof(entry->d_name) + 1 +sizeof('\n') );
		strcat(filename, entry->d_name);
		strcat(filename, "\n");
	}
	int size;
	size = strlen(filename);
	//size = htonl(size);
	
	REPLY = list_reply(filename, size);
	
	sendP_message(REPLY, client_sd);
	char listContent[size+1];
	listContent[0] = '\0';
	strcat(listContent, REPLY->payload);
	listContent[size] = '\0';
	//REPLY->length = htonl(REPLY->length);
	/*if((len=send(client_sd,listContent,size+1,0))<0){
		printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}*/
	
	printf("list  : %s\n",listContent);
	free(filename);
	free(REPLY);
}

void *client_action(void *sd){
	int client_sd = *((int *)sd);
	char filename[] = "fuck.txt";
	int filelen = 9;
	int len;
	P_message *REPLY = (P_message *) malloc(sizeof(P_message));
	P_message *REQUEST = (P_message *) malloc(sizeof(P_message));
	//LIST_REPLY = list_reply(fileName, filelen);
	//recv(client_sd,inputBuffer,sizeof(inputBuffer),0);
	
	//printf("Get:%s\n",inputBuffer);
	
	REQUEST = recvP_message(client_sd);
	/*if((len=recv(client_sd,REQUEST,sizeof(*REQUEST),0))<0){
		printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}*/
	
	if(REQUEST->type == 0xA1){
		printf("stype LIST    : %x\n",REQUEST->type);
		send_list(client_sd);
	}else if(REQUEST->type == 0xB1){
		printf("stype GET    : %x\n",REQUEST->type);
		if(list_compare(REQUEST->payload) == 1){
			printf("File found\n");
			///////////////send signal
			REPLY = get_reply(1);
			/*if((len=send(client_sd,REPLY,sizeof(*REPLY),0))<0){ 
				printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
				exit(0);
			}*/
			sendP_message(REPLY,client_sd);
			///////////////ready to send
			send_toClient(client_sd, REQUEST->payload);
			
		}else{
			printf("No such file\n");
			REPLY = get_reply(0);
			/*if((len=send(client_sd,REPLY,sizeof(*REPLY),0))<0){
				printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
				exit(0);
			}*/
			sendP_message(REPLY, client_sd);
		}
		
		//printf("send type: %x", LIST_REPLY->type);
		
	}else if(REQUEST->type == 0xC1){
		printf("stype PUT    : %x\n",REQUEST->type);
		REPLY = put_reply();
		//printf("send type: %x", LIST_REPLY->type);
		/*if((len=send(client_sd,REPLY,sizeof(*REPLY),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}*/
		sendP_message(REPLY, client_sd);
		
		recv_fromClient(client_sd);
		
		
	}else{
		printf("Fail\n");
	}

	print_debug(REQUEST);
	free(REQUEST);
	free(REPLY);
	pthread_exit(NULL);
}
