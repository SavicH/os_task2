#include "pop3.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define BUFSIZE 1024
#define LENGTH 32

int serve_client(int sock, pop3_data *data, int len) 
{
    char buf[BUFSIZE];
    int n;
    n = recv(sock, buf, 1024, 0);
    if (n == -1)
    {
        if (errno != EAGAIN)
        {
            perror ("read");
            close(sock);
            return 1;
        }
    }
    else
    {
        if (n == 0)
        {
           close(sock);
           return 0;
        }
    }

    char *command = strtok(buf, " ");
    char *msg = malloc(BUFSIZE);

    if (!strncmp(command, "APOP", 4))
    {
        char *username = strtok(NULL, " ");
        char *password = strtok(NULL, "\n");
        int ok = 0;
        int i;
        for (i = 0; i<len & !ok; i++)
        {
            ok = (strcmp(data[i].username, username) == 0) & (strcmp(data[i].password, password) == 0);
        }
        if (ok)
        {
            sprintf(msg, "+OK maildrop has %d message\n", data[i].count);
        }
        else
        {
            sprintf(msg, "-ERR password suplied for %s is incorrect\n", username);
        }
    } else
    if (!strncmp(command, "NOOP", 4))
    {
        sprintf(msg, "+OK\n");
    } else
    if (!strncmp(command, "STAT", 4))
    {
        sprintf(msg, "+OK a b\n");
    } else
    if (!strncmp(command, "QUIT", 4))
    {
        sprintf(msg, "+OK\n");
    } else
    {
        sprintf(msg, "Unknown command\n");
    }
    send(sock, msg, strlen(msg), 0);
    free(msg);        
    return 0;
}

char buf[BUFSIZE];
int position;

static char *get_token()
{
    char *tmp;
    tmp = (char*)malloc(LENGTH);
    int i = 0;
    do
    {
        tmp[i++] = buf[position];
    }
    while (buf[position]!= 0 & buf[position]!=10 & buf[position++]!=32);
    tmp[i-1]=0;
    return tmp;
}

int cache(char *filename, pop3_data **data, int *len)
{
    FILE *f = fopen(filename, "r");
    if (!f) 
    {
        return 1;
    }
    fgets(buf, sizeof(buf), f);
    *len = atoi(buf);
    *data = malloc(sizeof(pop3_data) * (*len));
    int i;
    for (i = 0; i<*len; i++)
    {
        if (fgets(buf, sizeof(buf), f))
        {
            char *tmp;
            position = 0;
            tmp = get_token();
            (*data)[i].username = tmp;
            tmp = get_token();
            (*data)[i].password = tmp;
            tmp = get_token();
            (*data)[i].size = atoi(tmp);
            tmp = get_token();
            (*data)[i].size = atoi(tmp);
        }
    }
    fclose(f);
    return 0;
}
