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
int portNum, sd;

int n, k, block_size;
int no_of_block;
int num_success_connection = 0;
char **address;
char **port;
char Ip[15];
struct timeval tv;
void recv_file(int sd);
void send_file(int sd, char *file);
int list_compare(char *fileRequest);
int server_connection();
void read_file();


int main(int argc, char** argv){
	int len;
	int i, j, x, y;
	int s;
	int max_sd = 0;
	char buf[1024];
	if(argc < 3 || argc > 5){
		printf("Command error\n");
		exit(0);
	}
	fd_set readSet;
	FD_ZERO( &readSet );
	read_file(argv);
	int connect_sd[n];
	for(i = 0;i<n; i++){
		connect_sd[i] = 0;
	}
	num_success_connection = server_connection(argv, connect_sd);
	for(i = 0; i < n; i++){
		if(connect_sd[i]){
			printf("Connect_sd %d with sd(%d)has been SUCCESS\n",i,connect_sd[i]);
			FD_SET(connect_sd[i], &readSet);
			if(max_sd < connect_sd[i]){
				max_sd = connect_sd[i];
			}
		}
		
	}
	printf("MaxSD: %d\n", max_sd);
	s = select( max_sd+1, &readSet, &readSet, 0, NULL );
	//s = 1;
	if ( s < 0 ) {
		perror("[-] select call failed");
		//close( s );
		exit( 1 );
	}else{
		printf("[-] select call Success");
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
		
		//char message[] = {"Hi there"};
		//send(sd,message,sizeof(message),0);
        for(i = 0; i < n; i++){
			if(connect_sd[i]){
				printf("Trying %d", i);
				if ( FD_ISSET( connect_sd[i], &readSet ) ) {
					P_message *LIST_REQUEST = list_request();
					print_debug(LIST_REQUEST);
					sendP_message(LIST_REQUEST, connect_sd[i]);
					/*if((len=send(sd,LIST_REQUEST,sizeof(*LIST_REQUEST),0))<0){
						printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
						exit(0);
					}*/
					LIST_REQUEST = recvP_message(connect_sd[i]);

					print_debug(LIST_REQUEST);
					printf("List DIR From SERVER %d IP:%s\n", connect_sd[i],port[i]);
					break;
				}
			}
		
		}
        
		/*if((len=recv(sd,buf,LIST_REQUEST->length+1,0))<0){
			printf("recv Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
		buf[LIST_REQUEST->length] = '\0'; 
		printf("list:\n%s",buf);*/
		
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
		/*if((len=send(sd,GET_REQUEST,sizeof(*GET_REQUEST),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}*/
		sendP_message(GET_REQUEST, sd);
		GET_REQUEST = recvP_message(sd);
		/*if((len=recv(sd,GET_REQUEST,sizeof(*GET_REQUEST),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}*/
		
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
			/*if((len=send(sd,PUT_REQUEST,sizeof(*PUT_REQUEST),0))<0){
				printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
				exit(0);
			}*/
			sendP_message(PUT_REQUEST, sd);
			PUT_REQUEST = recvP_message(sd);
			/*if((len=recv(sd,PUT_REQUEST,sizeof(*PUT_REQUEST),0))<0){
				printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
				exit(0);
			}*/

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

void read_file(char** argv){
	FILE *fp;
	char *line = NULL;
    size_t length = 0;
    ssize_t read;

    //fp = fopen(argv[1], "r");
	fp = fopen("clientconfig.txt", "r");
    if (fp == NULL){
    	printf("File does not exist.\n");
    	exit(0);
    }


	/**
    while ((read = getline(&line, &length, fp)) != -1) {
        //printf("Retrieved line of length %zu:\n", read);
        printf("%s", line);
    }**/
    //Get n
    if ((read = getline(&line, &length, fp)) != -1){
    	n = atoi(line);
    	printf("n = %d\n",n);
    }
    //Get k
    if ((read = getline(&line, &length, fp)) != -1){
    	k = atoi(line);
    	printf("k = %d\n",k);
    }
    //Get block_size
    if ((read = getline(&line, &length, fp)) != -1){
    	block_size = atoi(line);
    	printf("block_size = %d\n",block_size);
    }
	//char address[n][16], port[n][6];
	address = (char**)malloc(n *sizeof(char*));
	port = (char**)malloc(n *sizeof(char*));
	for(int x = 0; x<n;x++){
		address[x] = (char*)malloc(sizeof(char) * 16);
		memset(address[x],0,16);
		port[x] = (char*)malloc(sizeof(char) * 6);
		memset(port[x],0,6);
	}
	    int o = 0;
    while ((read = getline(&line, &length, fp)) != -1) {
        //printf("Retrieved line of length %zu:\n", read);
        printf("%s", line);
        int i, z = 0, index = 0;
        for(i = 0; i < read; i++){
        	if(line[i] == ':'){
        		z = 1;
        		address[o][i] = '\0';
        	}else if(z==0){
        		address[o][i] = line[i];
        	}else if (z==1){
        		port[o][index] = line[i];
        		index++;
        	}
        }
        port[o][index] = '\0';
        printf("address is %s\n",address[o]);
        printf("port is %s\n",port[o]);
        o++;
	}
	fclose(fp);
}

int server_connection(char** argv, int *connect_sd){
	printf("------------------Connection proccess BEGIN-----------------\n");
	int i;
	int j = 0;
	for(i = 0; i < n; i++){
		portNum = atoi(port[i]);
		sd=socket(AF_INET,SOCK_STREAM,0);
		strcpy(Ip,argv[1]);
		struct sockaddr_in server_addr;
		memset(&server_addr,0,sizeof(server_addr));
		server_addr.sin_family=AF_INET;
		server_addr.sin_addr.s_addr=inet_addr(address[i]);
		server_addr.sin_port=htons(portNum);
		
		if(connect(sd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
			printf("connection %s %s error: %s (Errno:%d)\n",address[i],port[i],strerror(errno),errno);
			//exit(0);
		}else{
			printf("		Success Connection %s %s \n", address[i],port[i]);
			connect_sd[i] = sd;
			j++;
		}
		
	}
	if(j >= k){
		printf("Successful Connection: %d\n", j);
	}else{
		printf("Only Connnected : %d\n\n", j);
		exit(0);
	}
	printf("------------------Connection proccess END---------------------\n");
	return j;
}

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
	/*if((len=send(sd,FILE_DATA,sizeof(*FILE_DATA),0))<0){
		printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}*/
	sendP_message(FILE_DATA, sd);
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
	char cwd[MAX];
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
	/*if((len=recv(sd,FILE_DATA,sizeof(*FILE_DATA),0))<0){
		printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}*/
	FILE_DATA = recvP_message(sd);
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


