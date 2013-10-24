#include "pop3.h"
#include <string.h>
#include <stdio.h>

#define MAX 1024

int serve_client(int sock, pop3_data *data, int len) 
{
	char buf[MAX];
    int bytes_read;
	bytes_read = recv(sock, buf, 1024, 0);
    if (bytes_read > 0)
    { 
    	char *msg = "OK\n";
    	send(sock, msg, sizeof(msg), 0);
    }
    else
    {
    	perror("read");
    	return 1;
    }
}

int cache(char *filename, pop3_data *data, int *len)
{
	return 0;
}