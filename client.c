#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX 1024
#define PORT 2558

char test[] = "USER test\n";
char buf[MAX];

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

    send(sock, test, sizeof(test), 0);
    recv(sock, buf, sizeof(test), 0);
    
    printf("%s\n", (char*)buf);
    close(sock);

    return 0;
}