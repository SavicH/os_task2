#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include "pop3.h"

#define PORT 2558
#define MAXEVENTS 32
#define MAXCONNECTIONS 1000
#define FILENAME "data.txt"

static int make_socket_non_blocking (int sfd)
{
    int flags, s;

    flags = fcntl (sfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror ("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl (sfd, F_SETFL, flags);
    if (s == -1)
    {
        perror ("fcntl");
        return -1;
    }

    return 0;
}

static int create_listener(int *listener)
{
    struct sockaddr_in addr;

    *listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
    {
        perror("socket");
        exit(2);
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(*listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(3);
    }
}


int main()
{
    int listener, epollfd;

    struct epoll_event event;
    struct epoll_event *events;

    pop3_data *data;
    int len;

    if (cache(FILENAME, data, &len))
    {
        perror("cache");
        exit(1);
    }


    create_listener(&listener);
    make_socket_non_blocking(listener);
    listen(listener, MAXCONNECTIONS);

    epollfd = epoll_create1(0);
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = listener;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listener, &event) == -1)
    {
        perror("epoll_ctl");
        exit(4);
    }

    events = calloc(MAXEVENTS, sizeof(struct epoll_event));
    
    while(1)
    {
        int i, n;
        n = epoll_wait(epollfd, events, MAXEVENTS, -1);
        for (i=0; i<n; i++)
        {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN)))
            {
                fprintf (stderr, "epoll error\n");
                close (events[i].data.fd);
                continue;
            }

            if (events[i].data.fd == listener)
            {
                while (1)
                {
                    int sock = accept(listener, NULL, NULL);
                    if (sock == -1)
                    {
                        break;
                    }
                    make_socket_non_blocking(sock);
                    struct epoll_event event;
                    event.data.fd = sock;
                    event.events = EPOLLIN | EPOLLET;
                    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &event) == -1)
                    {
                        perror("epoll_ctl");
                        exit(4);
                    }
                }
            }
            else
            {
                serve_client(events[i].data.fd, data, len);
            }
        }
    }
    
    return 0;
}