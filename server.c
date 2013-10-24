#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "pop3.h"

#define PORT 2558
#define FILENAME "data.txt"

int main()
{
    pop3_data *data;
    int len;
    if (cache(FILENAME, data, &len))
    {
        perror("cache");
        exit(1);
    }

    int sock, listener;
    struct sockaddr_in addr;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
    {
        perror("socket");
        exit(2);
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(3);
    }

    listen(listener, 5);
    
    while(1)
    {
        sock = accept(listener, NULL, NULL);
        if(sock < 0)
        {
            perror("accept");
            exit(4);
        }

        pid_t pid = fork();
        switch (pid)
        {
            case -1:
                perror("fork");
                break;
            case 0:
                close(listener);
                serve_client(sock, data, len);
                exit(0);
                break;
            default:
                close(sock);
                break;
        }   

    }
    
    return 0;
}