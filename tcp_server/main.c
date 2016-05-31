#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "mylog.h"
#include "server.h"
#include "client.h"

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
    memset(buf, 0, 8192);

    len = read(ec->fd, buf, 8192);
    //printf("user data is %d\n", in_param);
    if (len <=0) {
        printf("client close %d\n", ec->fd);
        close(ec->fd);
        ec->free = true;
    } else {
        printf("sock %d, len %d: %s\n", ec->fd, len, buf);
        write(ec->fd, buf, len);
    }

    return (0);
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

    return server(my_accept, echo);
}
