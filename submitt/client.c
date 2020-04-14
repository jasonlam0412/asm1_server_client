# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
# include <math.h>
# include <dirent.h>
# include <pthread.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/stat.h> 
# include <netinet/in.h>
# include <arpa/inet.h>
# include "myftp.h"
//# define PORT 12345
#define MAX 1000
#define MMAX 255
#define KMAX 255
#define NLENGTH 100
typedef unsigned char u8;
int portNum, sd;

int n, k, block_size;
int no_of_block;
int no_of_stripe;
int num_success_connection = 0;
char **address;
char **port;
char Ip[15];
struct timeval tv;
void recv_file(int sd);
void send_file(int *sd, char *file,fd_set readSet;);
int list_compare(char *fileRequest);
int server_connection();
void read_file();
void send_file_information(int sd, char *file);
static int gf_gen_decode_matrix_simple(u8 * encode_matrix,
				       u8 * decode_matrix,
				       u8 * invert_matrix,
				       u8 * temp_matrix,
				       u8 * decode_index, u8 * frag_err_list, int nerrs, int k,
				       int m);

int decode_data(int n, int k, Stripe *stripe, size_t block_size, int *work_node, u8 *error_node);
void print_matrix(unsigned char * encode_matrix, int m, int k);
void print_matrix(unsigned char * encode_matrix, int m, int k){
	int i, j;
	for(i = 0; i<m; i++){
		for(j=0; j<k; j++){
			printf("%d ", encode_matrix[i*m+j]);
		}
		printf("\n");
	}
}

Stripe *stripe_init(Stripe *stripe, int n, int k, int block_size,int no_of_block){
	int i;
	//Stripe *stripe = malloc(sizeof(Stripe));
    stripe->encode_matrix = malloc(sizeof(unsigned char) * (n * k));
	stripe->decode_matrix = malloc(sizeof(unsigned char) * (n * k));
	stripe->invert_matrix = malloc(sizeof(unsigned char) * (k * k));
	stripe->error_matrix = malloc(sizeof(unsigned char) * (k * k));
    stripe->table = malloc(sizeof(unsigned char) * (32 * k * (n-k)));
    stripe->blocks = malloc(sizeof(unsigned char **) * n);
    printf("%d\n",(int)sizeof(stripe->blocks));
    for (i = 0; i < n; i++){
    	printf("GOOD 2\n");
    	stripe->blocks[i] = malloc(sizeof(unsigned char *) * block_size);
		memset(stripe->blocks[i],0,block_size);
    }
	return stripe;
}

Stripe *stripe_memset(Stripe *stripe, int n, int k, int block_size,int no_of_block){
	int i;
	//Stripe *stripe = malloc(sizeof(Stripe));
    stripe->encode_matrix = memset(stripe->encode_matrix, 0, sizeof(unsigned char) * (n * k));
	stripe->decode_matrix = memset(stripe->decode_matrix,0,sizeof(unsigned char) * (n * k));
	stripe->invert_matrix = memset(stripe->invert_matrix, 0 ,sizeof(unsigned char) * (k * k));
	stripe->error_matrix = memset(stripe->error_matrix, 0,sizeof(unsigned char) * (k * k));
    stripe->table = memset(stripe->table, 0, sizeof(unsigned char) * (32 * k * (n-k)));
    //stripe->blocks = memset(stripe->blocks,0,sizeof(unsigned char **) * n);
    printf("%d\n",(int)sizeof(stripe->blocks));
    for (i = 0; i < n; i++){
    	printf("GOOD 2\n");
    	//stripe->blocks[i] = malloc(sizeof(unsigned char *) * block_size);
		memset(stripe->blocks[i],0,block_size);
    }
	return stripe;
}

void stripe_free(Stripe *stripe, int n, int k, int block_size,int no_of_block){
	int i;
	//Stripe *stripe = malloc(sizeof(Stripe));
    free(stripe->encode_matrix);
	//stripe->decode_matrix = malloc(sizeof(unsigned char) * (n * k));
	free(stripe->invert_matrix);
	free(stripe->error_matrix );
    free(stripe->table);
    
    printf("%d\n",(int)sizeof(stripe->blocks));
    for (i = 0; i < n; i++){
    	free(stripe->blocks[i]);
    }
	free(stripe->blocks);
	free(stripe);

}

int main(int argc, char** argv){
	int len;
	int i, j, x, y;
	int s;
	int max_sd = 0;
	char buf[1024];
	if(argc != 3 && argc != 4 ){
		if(argc < 3)
			printf("There is too few arguements.\n");
		else
			printf("There is too many arguements.\n");
		printf("Please enter \"./myftpclient clientconfig.txt <list|get|put> <file>\"\n");
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
		perror("[-] select call failed\n");
		//close( s );
		exit( 1 );
	}else{
		printf("[-] select call Success\n");
	}
	
	//int le=0x12345678;
	//printf("%x\n",le);
	//le=htonl(le);
	//printf("%x\n",le);
	
	if(strcmp(argv[2],"list")==0){
		if(argc == 3){
			printf("You are in list.\n");
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
		
	}else if(strcmp(argv[2],"get")==0){
		char file_request[50];
		if(argc > 3){
			strcpy(file_request, argv[3]);
			printf("%s\n", file_request);
			printf("%d\n", (int)strlen(file_request));
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
		print_debug(GET_REQUEST);
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
		
	}else if(strcmp(argv[2],"put")==0){
		char file_put[50];
		if(argc > 3){
			strcpy(file_put, argv[3]);
			printf("%s\n", file_put);
			printf("%d\n", strlen(file_put));
		}else{
			printf("Please enter file name\n");
		}
		
		
		if(list_compare(file_put)){
			for(i = 0; i < n; i++){
				if(connect_sd[i]){
					printf("Trying Send %d Server\n", i);
					if ( FD_ISSET( connect_sd[i], &readSet ) ) {
						P_message *PUT_REQUEST = put_request(file_put, strlen(file_put));
						print_debug(PUT_REQUEST);
						/*if((len=send(sd,PUT_REQUEST,sizeof(*PUT_REQUEST),0))<0){
							printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
							exit(0);
						}*/
						sendP_message(PUT_REQUEST, connect_sd[i]);
						PUT_REQUEST = recvP_message(connect_sd[i]);
						/*if((len=recv(sd,PUT_REQUEST,sizeof(*PUT_REQUEST),0))<0){
							printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
							exit(0);
						}*/

						print_debug(PUT_REQUEST);
						send_file_information(connect_sd[i], file_put);
						printf("Put to SERVER %d IP:%s\n", connect_sd[i],port[i]);
						//break;
					}
				}
			
			}
			
			send_file(connect_sd, file_put, readSet);
			

			
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

    fp = fopen(argv[1], "r");
	//fp = fopen("clientconfig.txt", "r");
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

void send_file(int *connect_sd, char *file, fd_set readSet){
	int i, j, x;
	char filename[60];
	filename[0] ='\0';
	int len;
	strcat(filename,file);
	printf("%s", filename);
	FILE *fp = fopen(filename,"rb");
	struct stat *filestat = (struct stat *)malloc(sizeof(struct stat));
	P_message *FILE_DATA = (P_message *) malloc(sizeof(P_message));
	FILE_DATA = file_data();
	char buf[block_size];
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
	no_of_stripe = (int)ceil((double)file_size/(block_size * k));
	printf("no_of_stripe = %d\n",no_of_stripe);
	no_of_block = no_of_stripe * k;
	x=0;
	/////////////////////////////////////////////////////////////
	for(x = 0; x<no_of_stripe;x++){
		int work_node_num = 0;
		int work_node[n];
		u8 error_list[n];
		u8 error_node[n-k];
		for(i = 0;i<n;i++){
			error_list[i] = 1;
		}
		Stripe *stripe = (Stripe*)malloc(sizeof(Stripe));
		stripe = stripe_init(stripe, n, k, block_size, no_of_block);
		for(j = 0;j < k; j++){
			if(!feof(fp)){
				numbytes = fread(buf, 1, block_size, fp);
				memcpy(stripe->blocks[j], buf, block_size);
				printf("fread %d bytes, ", numbytes);
			}
		}
		
		
		
		gf_gen_rs_matrix(stripe->encode_matrix, n, k);

		ec_init_tables(k, n - k, &stripe->encode_matrix[k*k], stripe->table);

		unsigned char** blocks_data = malloc(sizeof(unsigned char **) * n);
		for (i = 0; i < n; i++){
			printf("GOOD malloc encode_data\n");
			blocks_data[i] = malloc(sizeof(unsigned char *) * block_size);
			memset(blocks_data[i],0,block_size);
		}
		for (i = 0; i < n ; i++){
			blocks_data[i] = stripe->blocks[i];
		}
		ec_encode_data(block_size, k, n-k, stripe->table, stripe->blocks, &stripe->blocks[k]);
		//printf("encode_matrix:\n");
		//print_matrix(stripe->encode_matrix, n, k);
		/*if((len=send(sd,FILE_DATA,sizeof(*FILE_DATA),0))<0){
			printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}*/
		//sendP_message(FILE_DATA, sd);
		//print_debug(FILE_DATA);
		for(i = 0; i < n; i++){
			if(connect_sd[i]){
				printf("Trying %d", i);
				if ( FD_ISSET( connect_sd[i], &readSet ) ) {
					work_node[work_node_num++] = i;
					numbytes = write(connect_sd[i], stripe->blocks[i], block_size);
					printf("Sending %d byte to server %d\n",block_size, i);
					FILE *fp_block;
					char block_name[NLENGTH];
					memset(block_name, 0 ,NLENGTH);
					char num[10];
					sprintf(num, "%d", i);
					strcat(block_name, file);
					strcat(block_name, "_");
					strcat(block_name, num);
					printf("%s\n", block_name);
					fp_block = fopen(block_name, "w");
					//memset(buf,0,block_size);
					//numbytes = fread(buf, 1, block_size, fp);
					//printf("fread %d bytes, ", numbytes);
					//memcpy(stripe->blocks[j], buf, block_size);
					//fwrite(stripe->blocks[i], sizeof(char), block_size, fp_block);



					//stripe->encode_matrix = encode_data(n, k, stripe, block_size);
					fwrite(stripe->blocks[i], sizeof(char), block_size, fp_block);
					fclose(fp_block);
					
					
					printf("Work node: ");
					printf("%d\n",work_node[work_node_num-1]);
					
					
					
				}
			}
		
		}
		/////////////////////decode test/////////////////////////////
		printf("\nError node: ");
		for(i = 0; i<k; i++){
			error_list[work_node[i]] = 0;
		}
		j = 0;
		for(i = 0; i<n; i++){
			if(error_list[i]){
				error_node[j++] = i;
			}
			printf("%d ",error_node[i]);
		}
		printf("\n");
		decode_data(n, k, stripe, block_size, work_node, error_node);
		
		///////////////////////decode end////////////////////////////
	}
	/*while(!feof(fp)){
		numbytes = fread(buf, sizeof(char), sizeof(buf), fp);
		printf("fread %d bytes, ", numbytes);
		numbytes = write(sd, buf, numbytes);
		printf("Sending %d bytesn",numbytes);
		printf("\n");
	}*/
	
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
	printf("receviving...\n");
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

int decode_data(int n, int k, Stripe *stripe, size_t block_size, int *work_node, u8 *error_node){
	int i, j;
	unsigned char decode_index[255];
	stripe->table = memset(stripe->table, 0, sizeof(unsigned char) * (32 * k * (n-k)));
	/*int work_node[n];
	u8 error_list[n];
	u8 error_node[n-k];
	printf("Work node: ");
	for(i = 0;i<n;i++){
		error_list[i] = 1;
	}
	for(i = 0; i< k; i++){
		work_node[i] = i+3;
		//work_node[1] = 4;
		printf("%d %d ",work_node[0],work_node[1]);
	}
	printf("\nError node: ");
	for(j = 0; j < k;j++){
		error_list[work_node[j]] = 0;
	}
	for(i = 0, j=0; i<n-k;i++,j++){
		if(error_list[j]){
			error_node[i] = j;
			
		}
		
		//error_node[1] = 1;
		//error_node[2] = 2;
		printf("%d ",error_node[i]);
	}*/
	unsigned char** blocks_data2 = (unsigned char **)malloc(sizeof(unsigned char *) * n);
	for (i = 0; i < n; i++){
		printf("GOOD malloc decode_data\n");
		blocks_data2[i] = malloc(sizeof(unsigned char *) * block_size);
		memset(blocks_data2[i],0,block_size);
	}				
	printf("GOOD 1\n");
	int ret;
	ret = gf_gen_decode_matrix_simple(stripe->encode_matrix, stripe->decode_matrix,
		  stripe->invert_matrix, stripe->error_matrix, decode_index,
		  error_node, n-k, k, n);
	if (ret != 0) {
		printf("Fail on generate decode matrix\n");
		//return -1;
	}
	// Pack recovery array pointers as list of valid fragments
	for (i = 0; i < k; i++){
		blocks_data2[i] = stripe->blocks[decode_index[i]];
		printf("decode index: %d\n", decode_index[i]);
	}

	// Recover data
	ec_init_tables(k, n-k, stripe->decode_matrix, stripe->table);
	ec_encode_data(block_size, k, n-k, stripe->table, blocks_data2, &blocks_data2[k]);
	
	// Check that recovered buffers are the same as original
	printf(" check recovery of block {");
	for (i = 0; i < n-k; i++) {
		printf(" %d", error_node[i]);
		if (memcmp(blocks_data2[k+i], stripe->blocks[error_node[i]], block_size)) {
			printf(" Fail erasure recovery %d, frag %d\n", i, error_node[i]);
			//return -1;
		}
	}

	printf(" } done all: Pass\n");printf("decode_matrix:\n");
	print_matrix(stripe->decode_matrix, n, k);
	for(i = 0; i<n-k;i++){
		printf("errorblock: %d  ",error_node[i]);
		if (memcmp(blocks_data2[k+i],stripe->blocks[error_node[i]], block_size)) {
			printf(" Fail erasure recovery %d, frag %d\n", i, error_node[i]);
			printf("After decode:\n%s\n\n", blocks_data2[k+i]);
			printf("Before decode:\n%s\n\n", stripe->blocks[error_node[i]]);
			
			//return 0;
		}else{
			printf("success recover\n");
		}
	}

	return 1;
}

static int gf_gen_decode_matrix_simple(u8 * encode_matrix,
				       u8 * decode_matrix,
				       u8 * invert_matrix,
				       u8 * temp_matrix,
				       u8 * decode_index, u8 * frag_err_list, int nerrs, int k,
				       int m)
{
	int i, j, p, r;
	int nsrcerrs = 0;
	u8 s, *b = temp_matrix;
	u8 frag_in_err[MMAX];

	memset(frag_in_err, 0, sizeof(frag_in_err));

	// Order the fragments in erasure for easier sorting
	for (i = 0; i < nerrs; i++) {
		if (frag_err_list[i] < k)
			nsrcerrs++;
		frag_in_err[frag_err_list[i]] = 1;
	}
	for(i = 0; i < m+1;i++){
		printf("frag_in_err: %d",frag_in_err[i]);
	}
	// Construct b (matrix that encoded remaining frags) by removing erased rows
	for (i = 0, r = 0; i < k; i++, r++) {
		while (frag_in_err[r]){
			printf("          errornum: %d\n", r);
			r++;
		}
		for (j = 0; j < k; j++)
			b[k * i + j] = encode_matrix[k * r + j];
		decode_index[i] = r;
	}
	printf("error_matrix:\n");
	print_matrix(b, m, k);
	// Invert matrix to get recovery matrix
	if (gf_invert_matrix(b, invert_matrix, k) < 0)
		return -1;
	printf("invert_matrix:\n");
	print_matrix(invert_matrix, m, k);
	// Get decode matrix with only wanted recovery rows
	for (i = 0; i < nerrs; i++) {
		if (frag_err_list[i] < k)	// A src err
			for (j = 0; j < k; j++)
				decode_matrix[k * i + j] =
				    invert_matrix[k * frag_err_list[i] + j];
	}
	r = 0;
	while (frag_err_list[r]){
			printf("          errornum: %d\n", frag_err_list[r]);
			r++;
		}
	printf("decode_matrix:\n");
	print_matrix(decode_matrix, m, k);
	// For non-src (parity) erasures need to multiply encode matrix * invert
	for (p = 0; p < nerrs; p++) {
		if (frag_err_list[p] >= k) {	// A parity err
			for (i = 0; i < k; i++) {
				s = 0;
				for (j = 0; j < k; j++)
					s ^= gf_mul(invert_matrix[j * k + i],
						    encode_matrix[k * frag_err_list[p] + j]);
				decode_matrix[k * p + i] = s;
			}
		}
	}
	printf("decode_matrix:\n");
	print_matrix(decode_matrix, m, k);
	return 0;
}

void send_file_information(int sd, char *file){
	char filename[60];
	filename[0] ='\0';
	int len;
	strcat(filename,file);
	printf("%s", filename);
	FILE *fp = fopen(filename,"rb");
	struct stat *filestat = (struct stat *)malloc(sizeof(struct stat));
	P_message *FILE_DATA = (P_message *) malloc(sizeof(P_message));
	FILE_DATA = file_data();
	char buf[block_size];
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
	
	
	free(FILE_DATA);
	free(filestat);
	fclose(fp);
}

