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
# include <math.h>
# include "myftp.h"
# include "/home/timcheng/isa-l/include/erasure_code.h"

# define PORT 12345
//# define PATH_MAX

int list_compare(char *fileRequest);
void send_list(int client_sd);
void *client_action(void *client_sd);
void send_toClient(int client_sd, char *filename);
void recv_fromClient(int sd);
void ISA_test();
uint8_t* encode_data(int n, int k, Stripe *stripe, size_t block_size);

pthread_t *thread_id;
int thread_num = 0;

int main(int argc, char** argv){
	int len;
	char inputBuffer[256] = {};
	thread_id = (pthread_t *) malloc(sizeof(pthread_t));
	int sd=socket(AF_INET,SOCK_STREAM,0);
    long val = 1;
	if(argc != 2){
		printf("Fuck input port number\n");
		exit(1);
	}

	

	ISA_test();



	int port_num = atoi(argv[1]);
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
	server_addr.sin_port=htons(port_num);
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
	if((len=send(client_sd,FILE_DATA,sizeof(*FILE_DATA),0))<0){
		printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}
	
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
	if((len=recv(sd,FILE_DATA,sizeof(*FILE_DATA),0))<0){
		printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}
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
        printf("%s",filename);
        printf("--------------------------\n");
	}
	printf("%s",filename);
	int size;
	size = strlen(filename);
	//size = htonl(size);
	
	REPLY = list_reply(filename, size);
	//REPLY->length = htonl(REPLY->length);
    //REPLY->type = htonl(REPLY->type);
	if((len=send(client_sd,REPLY,sizeof(*REPLY),0))<0){
		printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}
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
	
	
	if((len=recv(client_sd,REQUEST,sizeof(*REQUEST),0))<0){
		printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}
	print_debug(REQUEST);
    //REQUEST->type = ntohl(REQUEST->type);
    printf("16 = %d\n",ntohl(16));
    print_debug(REQUEST);
    
	
	if(REQUEST->type == 0xA1){
		printf("stype LIST    : %x\n",REQUEST->type);
		send_list(client_sd);
	}else if(REQUEST->type == 0xB1){
		printf("stype GET    : %x\n",REQUEST->type);
		if(list_compare(REQUEST->payload) == 1){
			printf("File found\n");
			///////////////send signal
			REPLY = get_reply(1);
			if((len=send(client_sd,REPLY,sizeof(*REPLY),0))<0){ 
				printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
				exit(0);
			}
			///////////////ready to send
			send_toClient(client_sd, REQUEST->payload);
			
		}else{
			printf("No such file\n");
			REPLY = get_reply(0);
            strcpy(REPLY->payload , " ");
			if((len=send(client_sd,REPLY,sizeof(*REPLY),0))<0){
				printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
				exit(0);
			}
		}
		
		//printf("send type: %x", LIST_REPLY->type);
		
	}else if(REQUEST->type == 0xC1){
		printf("stype PUT    : %x\n",REQUEST->type);
		REPLY = put_reply();
		//printf("send type: %x", LIST_REPLY->type);
		if((len=send(client_sd,REPLY,sizeof(*REPLY),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
		recv_fromClient(client_sd);
		
		
	}else{
		printf("Fail\n");
	}
	
	print_debug(REQUEST);
    print_debug(REPLY);

	free(REQUEST);
	free(REPLY);
	pthread_exit(NULL);
}

void ISA_test(){
	FILE *fp;
	char *line = NULL;
    size_t length = 0;
    ssize_t read;

    fp = fopen("serverconfig.txt", "r");
    if (fp == NULL){
    	printf("File does not exist.\n");
    	exit(0);
    }

    int n, k, ID, block_size, port_number;

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

    char file_name[] = "data/test.txt";
    fp = fopen(file_name, "r");
    long int res = 0;
    if(fp==NULL){
    	printf("test.txt does not exist\n");
    }else{
    	fseek(fp, 0L, SEEK_END);
    	res = ftell(fp);
    	
    	printf("size of the file is : %ld\n",res);
    }
    if(res>0){
 		int no_of_stripe = (int)ceil((double)res/(block_size * k));
 		printf("no_of_stripe = %d\n",no_of_stripe);
    }
    
    Stripe *stripe = malloc(sizeof(Stripe));
    stripe->encode_matrix = malloc(sizeof(unsigned char) * (n * k));
    stripe->table = malloc(sizeof(unsigned char) * (32 * k * (n-k)));
    stripe->blocks = malloc(sizeof(unsigned char **) * n);
    printf("%d\n",(int)sizeof(stripe->blocks));
    for (int i = 0; i < n; i++){
    	 printf("GOOD 2\n");
    	stripe->blocks[i] = malloc(sizeof(unsigned char *));
    }
    
    fseek(fp, 0, SEEK_SET);
    char buf[block_size];
    int x = 0,numbytes;

    while(!feof(fp)){
		numbytes = fread(buf, sizeof(char), sizeof(buf), fp);
		printf("fread %d bytes, ", numbytes);
		stripe->blocks[x]->data = buf;
		x++;
	}

    /*uint8_t *encode_matrix = malloc(sizeof(uint8_t) * (n * k));
	uint8_t *errors_matrix = malloc(sizeof(uint8_t) * (n * k));
	uint8_t *invert_matrix = malloc(sizeof(uint8_t) * (n * k));
	uint8_t *table = malloc(sizeof(uint8_t) * (32 * k * (n-k)));*/

	encode_data(n, k, stripe, block_size);

}

uint8_t* encode_data(int n, int k, Stripe *stripe, size_t block_size){
	printf("GOOD 1\n");

	gf_gen_rs_matrix(stripe->encode_matrix, n, k);

	ec_init_tables(k, n - k, &stripe->encode_matrix[k*k], stripe->table);

	unsigned char** blocks_data = malloc(sizeof(unsigned char **) * n);

	for (int i = 0; i < n ; i++){
		blocks_data[i] = stripe->blocks[i]->data;
	}
	ec_encode_data(block_size, k, n-k, stripe->table, blocks_data, &blocks_data[k]);

	return stripe->encode_matrix;
}