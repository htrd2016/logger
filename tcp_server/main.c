#include <stdio.h>
#include <stdlib.h>
#include <error.h>

#include "memtypes.h"
#include "mylog.h"
#include "server.h"
#include "epollclient.h"

ConfData configData;

int my_accept(int connfd, struct sockaddr_in* cliaddr, void** out_data) {
    (void)connfd;
    (void)cliaddr;
    (void)out_data;

    printf("Accept a socket %d\n", connfd);
    return (0);
}

int echo (void* in_param) {
    EpollClient* ec = (EpollClient*)in_param;

    char buf[8192];
    int len = 0;
    len = read(ec->fd, buf, 8192);
    printf("sock %d, read len %d, error is %d\n", ec->fd, len, errno);
    if (len <= 0) {
        printf("read error %d (%d)\n", ec->fd, errno);
        return -1;
    }
    len = write(ec->fd, buf, len);
    printf("sock %d, write len %d\n", ec->fd, len);
    if (len <= 0) return -1;
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: tcp_server port block_amount\n");
        return (0);
    }

    memset(&configData, 0, sizeof(configData));
    configData.local_port = atoi(argv[1]);
    configData.block_amount = atoi(argv[2]);
    strcpy(configData.logfile, "/home/sean/tcpserver.log");
    return server(my_accept, echo);
}
