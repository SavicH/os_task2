#include "pop3.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define MAX 1024

int serve_client(int sock, pop3_data *data, int len) 
{
    int done = 0;
    char buf[MAX];
    int n;
    //while (1) 
    //{
    n = recv(sock, buf, 1024, 0);
    if (n == -1)
    {
        if (errno != EAGAIN)
        {
            perror ("read");
            done = 1;
        }
        //break;
    }
    else
    {
        if (n == 0)
        {
           done = 1;
           //break;
        }
    }
    //}

    if (done)
    {
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
    FILE *f = fopen(filename, "r");
    if (!f) 
    {
        return 1;
    }
    char buf[MAX];
    fgets(buf, sizeof(buf), f);
    *len = atoi(buf);
    data = calloc(sizeof(pop3_data), *len);
    int i;
    for (i = 0; i<*len; i++)
    {
        if (fgets(buf, sizeof(buf), f))
        {
            data[i].username = strtok(buf, " ");
            data[i].password = strtok(NULL, " ");
            data[i].count = atoi(strtok(NULL, " "));
            data[i].size = atoi(strtok(NULL, " "));
        }
    }
    fclose(f);
    return 0;
}