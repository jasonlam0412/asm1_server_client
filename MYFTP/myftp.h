typedef struct message_s {
    unsigned char protocol[6];
    unsigned char type;
    unsigned int length;
	unsigned char payload[5];
	
} __attribute__ ((packed)) P_message;

P_message *list_request();
P_message *list_reply(char *filename, int filelength);
P_message *get_request(char *filename, int filelength);
P_message *get_reply(int exist);
P_message *put_request(char *filename, int filelength);
P_message *put_reply();
P_message *file_data(char *filename, int filelength);