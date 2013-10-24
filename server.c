#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAX 1024
#define PORT 2558

int main()
{
    int sock, listener;
    struct sockaddr_in addr;
    char buf[MAX];
    int bytes_read;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
    {
        perror("socket");
        exit(1);
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(2);
    }

    listen(listener, 5);
    
    while(1)
    {
        sock = accept(listener, NULL, NULL);
        if(sock < 0)
        {
            perror("accept");
            exit(3);
        }

        pid_t pid = fork();
        switch (pid)
        {
            case -1:
                perror("fork");
                break;
            case 0:
                close(listener);
                bytes_read = recv(sock, buf, 1024, 0);
                if(bytes_read <= 0) break;
                char *msg = "OK\n";
                send(sock, msg, bytes_read, 0);
                exit(0);
                break;
            default:
                close(sock);
                break;
        }   

    }
    
    return 0;
}