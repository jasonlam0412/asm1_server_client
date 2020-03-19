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
# include "myftp.h"
# define PORT 12345
# define PATH_MAX 1000
int portNum, sd;
char Ip[15];
void recv_file(int sd);
void send_file(int sd, char *file);
int list_compare(char *fileRequest);

void send_file(int sd, char *file){
	char filename[60];
	filename[0] ='\0';
	int len;
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
	if((len=send(sd,FILE_DATA,sizeof(*FILE_DATA),0))<0){
		printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}
	print_debug(FILE_DATA);
	while(!feof(fp)){
		numbytes = fread(buf, sizeof(char), sizeof(buf), fp);
		printf("fread %d bytes, ", numbytes);
		numbytes = write(sd, buf, numbytes);
		printf("Sending %d bytesn",numbytes);
		printf("\n");
	}
	
	free(FILE_DATA);
	free(filestat);
	fclose(fp);
}

int list_compare(char *fileRequest){
	DIR *folder;
	struct dirent *entry;
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));
	strcat(cwd, "/.");
	folder = opendir(cwd);
	//char *filename = NULL;
	int bingo = 0;
	while((entry = readdir(folder)) != NULL){
		if(strcmp(entry->d_name, fileRequest) == 0){
			bingo =1;
			break;
		}
		//printf("%s\n", entry->d_name);
	}
	
	return bingo;
	
}

void recv_file(int sd){
	P_message *FILE_DATA = (P_message *) malloc(sizeof(P_message));
	FILE_DATA = file_data();
	int len;
	if((len=recv(sd,FILE_DATA,sizeof(*FILE_DATA),0))<0){
		printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}
	FILE *fp = fopen(FILE_DATA->payload, "wb");
	int numbytes;
	char buf[1024];
	print_debug(FILE_DATA);
	int fileSize = FILE_DATA->length;
	do{
		numbytes = read(sd, buf, sizeof(buf));
		printf("%d\n", sizeof(buf));
		printf("read %d bytes, ", numbytes);
		if(numbytes == 0){
			//break;
		}
		numbytes = fwrite(buf, sizeof(char), numbytes, fp);
		printf("fwrite %d bytesn", numbytes);
		fileSize -= sizeof(buf);
	}while(fileSize>0);
	free(FILE_DATA);
	fclose(fp);
}

int main(int argc, char** argv){
	int len;
	char buf[1024];
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
	server_addr.sin_addr.s_addr=inet_addr(Ip);
	server_addr.sin_port=htons(portNum);
	
	if(connect(sd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
		printf("connection error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}
	//int le=0x12345678;
	//printf("%x\n",le);
	//le=htonl(le);
	//printf("%x\n",le);
	
	if(strcmp(argv[3],"list")==0){
		if(argc == 4){
			
		}else{
			printf("Input too more\n");
			exit(0);
		}
		P_message *LIST_REQUEST = list_request();
		//char message[] = {"Hi there"};
		//send(sd,message,sizeof(message),0);
        
        print_debug(LIST_REQUEST);
        //LIST_REQUEST->type = htons(LIST_REQUEST->type);
        printf("0xA1 = %d\n",(unsigned char)htons(0xA1));
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
		
		if(GET_REQUEST->type == 0xB2){  /////            file exist
			
			recv_file(sd);
			
		}else if(GET_REQUEST->type == 0xB3){ ////////////    file not found
			printf("File not found\n");
		}else{
			printf("GET_REQUEST->type ERROR\n");
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
		
		
		if(list_compare(file_put)){
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
			send_file(sd, file_put);
			
		}else{
			printf("No such file\n");
			exit(0);
		}
			
		
		
		
		
		
	}else{
		printf("Only input list/get/put\n");
		exit(0);
	}
	//free(buf);
	close(sd);
	return 0;
}
