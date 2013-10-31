#ifndef POP3_H
#define POP3_H

#include <sys/socket.h>

struct pop3_data_s
{
	char *username;
	char *password;
	int count;
	int size;
};

typedef struct pop3_data_s pop3_data;

int serve_client(int sock, pop3_data *data, int len);
int cache(char *filename, pop3_data *data, int *len);

#endif