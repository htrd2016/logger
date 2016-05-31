#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "mylog.h"
#include "server.h"

ConfData configData;

int my_accept(int connfd, struct sockaddr_in* cliaddr, void** out_data) {
    (void)connfd;
    (void)cliaddr;

    *out_data = 1010101;
    printf("Accept a socket %d\n", connfd);
    return (0);
}

int echo (int connfd, struct sockaddr_in* cliaddr, void* in_param) {
    (void)cliaddr;
    (void)in_param;

    char buf[8192];
    int len = 0;
    memset(buf, 0, 8192);

    len = read(connfd, buf, 8192);
    printf("user data is %d\n", (int)in_param);
    if (len <=0) {
        printf("client close %d\n", connfd);
        close(connfd);
    } else {
        printf("sock %d, len %d: %s\n", connfd, len, buf);
        write(connfd, buf, len);
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
    configData.block_amount = atoi(argv[2]);
    configData.local_port = atoi(argv[1]);

    return server(my_accept, echo);
}
