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
#include <getopt.h>
//#include "erasure_code.h"	// use <isa-l.h> instead when linking against installed
#include <isa-l.h>
#define MMAX 255
#define KMAX 255
#define NLENGTH 100
typedef unsigned char u8;
int portNum, sd;
int n, k, block_size;
int no_of_block;
char **address;
char **port;
char Ip[15];
static int gf_gen_decode_matrix_simple(u8 * encode_matrix,
				       u8 * decode_matrix,
				       u8 * invert_matrix,
				       u8 * temp_matrix,
				       u8 * decode_index, u8 * frag_err_list, int nerrs, int k,
				       int m);
uint8_t* encode_data(int n, int k, Stripe *stripe, size_t block_size);
int decode_data(int n, int k, Stripe *stripe, size_t block_size);
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

void read_file(char** argv){
	int x;
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
	for(x = 0; x<n;x++){
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


void isa_test(){
	Metadata *metadata = (Metadata*)malloc(sizeof(Metadata));
	FILE *fp;
	FILE *fp_write;
	char *line = NULL;
    size_t length = 0;
    ssize_t read_file;
	int i , j, y, p, r;
	char s;
	int no_of_block;
	int no_of_stripe;
    fp = fopen("serverconfig.txt", "r");
    if (fp == NULL){
    	printf("File does not exist.\n");
    	exit(0);
    }

    int n, k, ID, block_size, port_number;
	unsigned char decode_index[255];
    int o = 0;
    int temp_input[5];

    while((read_file = getline(&line, &length, fp)) != -1){
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

    char file_name[] = "data/tuto2.pdf";
	char file_name_write[] = "tuto2.pdf";
    fp = fopen(file_name, "r");
	fp_write = fopen(file_name_write, "w");
	
    long int res = 0;
    if(fp==NULL){
    	printf("test.txt does not exist\n");
    }else{
    	fseek(fp, 0L, SEEK_END);
    	res = ftell(fp);
    	metadata->file_size = res;
    	printf("size of the file is : %ld\n",res);
    }
    if(res>0){
 		no_of_stripe = (int)ceil((double)res/(block_size * k));
		metadata->num_stripe = no_of_stripe;
 		printf("no_of_stripe = %d\n",no_of_stripe);
		no_of_block = no_of_stripe * k;
		metadata->num_block = no_of_block;
    }
    
    
	Stripe *stripe = (Stripe*)malloc(sizeof(Stripe));
	stripe = stripe_init(stripe, n, k, block_size, no_of_block);
	fseek(fp, 0, SEEK_SET);
    char buf[block_size];
	
    int x = 0,numbytes;
	i = 0;
	//for(i = 0; i < no_of_stripe;i++){
		stripe = stripe_memset(stripe, n, k, block_size, no_of_block);
		printf("%d\n ", i);
		for(j = 0;j < k; j++){
			if(!feof(fp)){
				FILE *fp_block;
				char block_name[NLENGTH];
				memset(block_name, 0 ,NLENGTH);
				char num[10];
				sprintf(num, "%d", k*i +j);
				strcat(block_name, file_name_write);
				strcat(block_name, "_");
				strcat(block_name, num);
				printf("%s\n", block_name);
				fp_block = fopen(block_name, "w");
				memset(buf,0,block_size);
				numbytes = fread(buf, 1, block_size, fp);
				printf("fread %d bytes, ", numbytes);
				memcpy(stripe->blocks[j], buf, block_size);
				//fwrite(stripe->blocks[j], sizeof(char), block_size, fp_block);
				
				
				
				//stripe->encode_matrix = encode_data(n, k, stripe, block_size);
				fwrite(stripe->blocks[j], sizeof(char), block_size, fp_write);
				fclose(fp_block);
			}
		}
		
		////////////////encode/////////////////////
				
				stripe->encode_matrix = encode_data(n,  k, stripe, block_size);
				printf("encode_matrix:\n");
				print_matrix(stripe->encode_matrix, n, k);
				///////////////decode/////////////////////////
				
				decode_data(n, k, stripe, block_size);
				//////////////////////////////////////////////
		
		//stripe_free(stripe, n, k, block_size, no_of_block);
		//free(stripe);
	//}
	//printf("no_of_stripe = %d\n",no_of_stripe);
	//printf("no_of_block = %d\n",no_of_block);
   // Stripe *stripe = malloc(sizeof(Stripe));
    //stripe = stripe_init(stripe, n, k, block_size, no_of_block);
    
    /*fseek(fp, 0, SEEK_SET);
    char buf[block_size];
    int x = 0,numbytes;

    while(!feof(fp)){
		memset(buf,0,block_size);
		numbytes = fread(buf, 1, block_size, fp);
		//printf("fread %d bytes, ", numbytes);
		
		memcpy(stripe->blocks[x], buf, block_size);
		//fwrite(stripe->blocks[x], sizeof(char), block_size, fp_write);
		x++;
		
	}
	printf("%d\n",x);
	printf("%d\n",(int)strlen(stripe->blocks[x-1]));
    for(i = 0; i<x;i++){
		fwrite(stripe->blocks[i], sizeof(char), block_size, fp_write);
		
	}*/
	//encode_data(n, k, stripe, block_size);
	fclose(fp);
	fclose(fp_write);
}

uint8_t* encode_data(int n, int k, Stripe *stripe, size_t block_size){
	int i;
	printf("GOOD 1\n");

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
	ec_encode_data(block_size, k, n-k, stripe->table, blocks_data, &blocks_data[k]);

	return stripe->encode_matrix;
}
int decode_data(int n, int k, Stripe *stripe, size_t block_size){
	int i, j;
	unsigned char decode_index[255];
	stripe->table = memset(stripe->table, 0, sizeof(unsigned char) * (32 * k * (n-k)));
	int work_node[n];
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
	}
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

int main(int argc, char** argv){
	//read_file(argv);
	isa_test();
	return 0;
}