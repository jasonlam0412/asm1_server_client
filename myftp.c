# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <arpa/inet.h>




P_message *list_request(){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	stpcpy(p_message->protocol, "myftp");
	p_message->type = 0xA1;
	p_message->length = 0;
	return p_message;
}

P_message *list_reply(char *filename, int filelength){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	stpcpy(p_message->protocol, "myftp");
	p_message->type = 0xA1;
	p_message->length = 0 + filelength;
	strcpy(p_message->payload,filename);
	return p_message;
}

P_message *get_request(char *filename, int filelength){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	stpcpy(p_message->protocol, "myftp");
	p_message->type = 0xA1;
	p_message->length = 0 + filelength;
	strcpy(p_message->payload,filename);
	return p_message;
}


P_message *get_reply(int exist){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	stpcpy(p_message->protocol, "myftp");
	if(exist){
		p_message->type = 0xB2;
	}else{
		p_message->type = 0xB30;
	}
	
	p_message->length = 0;
	return p_message;
}

P_message *put_request(char *filename, int filelength){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	stpcpy(p_message->protocol, "myftp");
	p_message->type = 0xA1;
	p_message->length = 0 + filelength;
	strcpy(p_message->payload,filename);
	return p_message;
}

P_message *put_reply(){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	stpcpy(p_message->protocol, "myftp");
	p_message->type = 0xA1;
	p_message->length = 0;
	return p_message;
}

P_message *file_data(char *filename, int filelength){
	P_message *p_message = (P_message *)malloc(sizeof(P_message));
	stpcpy(p_message->protocol, "myftp");
	p_message->type = 0xA1;
	p_message->length = 0 + filelength;
	strcpy(p_message->payload,filename);
	return p_message;
}