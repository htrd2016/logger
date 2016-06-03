#include <stdio.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_EVENTS 1024
#define BUFFER_LENGTH 8192

void epoll_server()
{
    int listenq = 1024;
    int listenfd, n;
    struct sockaddr_in servaddr, cliaddr;
    struct epoll_event ev, events[MAX_EVENTS];
    char buf[BUFFER_LENGTH];
    int conn_sock, nfds, epollfd;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(12349);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        printf("create cocket error(%d)!!", errno);
        return;
    }

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind error");
        printf("socket bind error(%s)!!", strerror(errno));
        return;
    }

    if (listen(listenfd, listenq) == -1)
    {
        printf("socket listen error(%s)!!", strerror(errno));
        return;
    }

    fcntl(listenfd, F_SETFL, O_NONBLOCK);

    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN|EPOLLET;
    ev.data.fd = listenfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) == -1)
    {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    for (;;)
    {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (n = 0; n < nfds; ++n)
        {
            if (events[n].data.fd == listenfd)
            {
                conn_sock = accept(listenfd, (struct sockaddr *)&cliaddr, &addrlen);
                if (conn_sock == -1)
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }

                if (fcntl(conn_sock, F_SETFL, fcntl(conn_sock, F_GETFD, 0) | O_NONBLOCK) == -1)
                {
                    printf("error %d\n", errno);
                    return;
                }

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
                              &ev) == -1) {
                    perror("epoll_ctl: conn_sock");
                    exit(EXIT_FAILURE);
                }
            }
            else if((events[n].events&EPOLLERR)
                    || (events[n].events&EPOLLRDHUP)
                    || (events[n].events&EPOLLHUP))//error
            {
                perror("soc: conn_sock");
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = events[n].data.fd;
                close(events[n].data.fd);
                epoll_ctl(events[n].data.fd, EPOLL_CTL_DEL, events[n].data.fd, &ev);
                printf("error %d\n", errno);
                continue;
            }
            else
            {
                ssize_t len = read(events[n].data.fd,buf, BUFFER_LENGTH);
                //if (len==0)
                //{
                //              //      perror("soc: conn_sock");
                //              //      ev.events = EPOLLIN | EPOLLET;
                //              //      ev.data.fd = events[n].data.fd;
                //              //      close(events[n].data.fd);
                //              //      epoll_ctl(events[n].data.fd, EPOLL_CTL_DEL, events[n].data.fd, &ev);
                //    printf("error %d\n", errno);
                //
                //    perror("read non data\n");
                //    continue;
                //}else
                if(len<=0 && errno != EINTR)//read error
                {
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = events[n].data.fd;
                    close(events[n].data.fd);
                    epoll_ctl(events[n].data.fd, EPOLL_CTL_DEL, events[n].data.fd, &ev);
                    printf("error %d\n", errno);
                    continue;
                    //exit(EXIT_FAILURE);
                }
                printf("recv len=%d\n",(int)len);
                write(events[n].data.fd, buf, BUFFER_LENGTH);
                //do_use_fd(events[n].data.fd);
            }
        }
    }
}

int main(void)
{
    epoll_server();
    return 0;
}

