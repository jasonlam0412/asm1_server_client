typedef struct message_s {
    unsigned char protocol[6];
    unsigned char type;
    unsigned int length;
	unsigned char payload[500];
	
} __attribute__ ((packed)) P_message;



typedef struct stripe {
	int sid;	// stripe id
	//unsigned char **data_block;	// pointer to the first data block
	//unsigned char **parity_block;	// pointer to the first parity block
	unsigned char **blocks;
	unsigned char *encode_matrix;
	unsigned char *table;
} Stripe;

P_message *list_request();
P_message *list_reply(char *filename, int filelength);
P_message *get_request(char *filename, int filelength);
P_message *get_reply(int exist);
P_message *put_request(char *filename, int filelength);
P_message *put_reply();
P_message *file_data();
void print_debug(P_message *p_message);
