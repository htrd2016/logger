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
    while((len = read(ec->fd, buf, 8192) > 0)) {
        //printf("sock %d, read len %d, error is %d, %s\n", ec->fd, len, errno, strerror(errno));
        if (len < 0) break;
        len = write(ec->fd, buf,len);
        if (len < 0) break;
    }

    shutdown(ec->fd, SHUT_RDWR);
    close(ec->fd);


    return (-1); //close
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
    strcpy(configData.logfile, "tcpserver.log");
    return server(my_accept, echo);
}
