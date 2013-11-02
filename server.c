#include <syslog.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "server.h"
#include "pop3.h"

#define PORT 2558
#define MAXEVENTS 4
#define MAXCONNECTIONS 1000

#define READPIPE "/tmp/readpipe"
#define WRITEPIPE "/tmp/writepipe"

void daemonize(const char *cmd)
{
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    umask(0);

    if (getrlimit(RLIMIT_NOFILE, &rl) <0)
    {
        printf("%s error", cmd);
        exit(1);
    }

    if ((pid = fork())<0)
    {
        printf("%s error", cmd);
        exit(1);
    }   
    else 
    {
        if (pid != 0)
        {
            exit(0);
        }
    }
    setsid();

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        printf("%s error", cmd);
        exit(1);
    }
    if ((pid = fork())<0)
    {
        printf("%s error", cmd);
        exit(1);
    }
    else 
    {
        if (pid != 0)
        {
            exit(0);
        }
    }
    if (chdir("/") < 0)
    {
        printf("%s error", cmd);
        exit(1);
    }
    if (rl.rlim_max == RLIM_INFINITY)
    {
        rl.rlim_max = 1024;
    }
    for (i=0; i< rl.rlim_max; i++)
    {
        close(i);
    }

    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        syslog(LOG_ERR, "error file descriptors %d %d %d", fd0, fd1, fd2);
        exit(1);
    }
}

int make_socket_non_blocking (int sfd)
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

int create_listener(int *listener)
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

int file_exists(char * filename)
{
    struct stat info;
    if(!stat(filename ,&info)) {
        if(errno == ENOENT) {
            return 0;
        } else if(errno == EACCES) {
            return 0;
        } else {
            return 1;
        }
    }
    else
    {
        return 0;
    }
}

void prefork()
{
    int i;
    for (i = 0; i<MAXEVENTS; i++)
    {   
        if (!fork())
        {
            printf("%s\n", "I'm here!");
            while (1)
            {
                sleep(30);
            }
        }
    }
}

int main(int argc, char* argv[])
{
    //daemonize("POP3 server");

    pop3_data *data;
    int len;
    if (cache(argv[1], &data, &len))
    {
        perror("cache");
        exit(1);
    }

    if (!file_exists(READPIPE))
    {
        if (mkfifo(READPIPE, 0666))
        {
            perror("pipe");
            exit(1);
        }
    }

    if (!file_exists(WRITEPIPE))
    {
        if (mkfifo(WRITEPIPE, 0666))
        {
            perror("pipe");
            exit(1);
        }
    }

    prefork();

    FILE *readpipe, *writepipe;

    //readpipe = open(READPIPE, O_WRONLY);
    //writepipe = open(WRITEPIPE, O_RDONLY);

    // close(readpipe);
    // readpipe = open(READPIPE, O_RDONLY | O_NONBLOCK);

    // close(writepipe);
    // writepipe = open(WRITEPIPE, O_WRONLY);

    int listener, epollfd;

    struct epoll_event event;
    struct epoll_event *events;

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
                    char *msg = "+OK POP3 server ready\n";
                    send(sock, msg, strlen(msg), 0); 
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
    fclose(readpipe);
    fclose(writepipe);
    return 0;
}