typedef struct message_s {
    unsigned char protocol[5];
    unsigned char type;
    unsigned int length;
	unsigned char payload[50];
	
} __attribute__ ((packed)) P_message;

P_message *list_request();
P_message *list_reply();
P_message *get_request();
P_message *get_reply();
P_message *put_request();
P_message *put_reply();
P_message *file_data();