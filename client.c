#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>

#define MAX 1024
#define PORT 2558

char buf[MAX];
char command[MAX];

int main()
{
    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        perror("socket");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        return 2;
    }
    recv(sock, buf, sizeof(buf), 0);
    printf("%s\n", (char*)buf);
    while (1) 
    {
        fgets(command, sizeof(command), stdin);
        send(sock, command, strlen(command), 0);
        memset(buf, 0, sizeof(buf));
        recv(sock, buf, sizeof(buf), 0);
        printf("%s\n", (char*)buf);
        if (strstr(command, "QUIT"))
        {
            break;    
        }
    }
    send(sock, command, 0, 0);    
    close(sock);

    return 0;
}