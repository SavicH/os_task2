#include "pop3.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define MAX 1024

int serve_client(int sock, pop3_data *data, int len) 
{
    int done = 0;
    char buf[MAX];
    int bytes_read;
    while (1) 
    {
        bytes_read = recv(sock, buf, 1024, 0);
        if (bytes_read == -1)
        {
            if (errno != EAGAIN)
            {
                perror ("read");
                done = 1;
            }
            break;
        }
        else
        {
            if (bytes_read == 0)
            {
            done = 1;
            break;
            }
        }
    }

    if (done)
    {
        //char *msg = "OK\n";
        //send(sock, msg, sizeof(msg), 0);  
        close(sock);
    }
    else
    {
        char *msg = "OK\n";
        send(sock, msg, sizeof(msg), 0);        
    }
}

int cache(char *filename, pop3_data *data, int *len)
{
    return 0;
}