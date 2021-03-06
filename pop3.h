#ifndef POP3_H
#define POP3_H

struct pop3_data_s
{
	char *username;
	char *password;
	int count;
	int size;
};

typedef struct pop3_data_s pop3_data;

char* serve_client(char *buf, pop3_data *data, int len);
int cache(char *filename, pop3_data **data, int *len);

#endif