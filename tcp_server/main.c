#include <stdio.h>
#include <stdlib.h>

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
    memset(buf, 0, 8192);
    char *ptr = buf;
    ssize_t readLen = 0;
    ssize_t remaning_length = 8192;

    while(remaning_length>0)
    {
        len = read(ec->fd, ptr, remaning_length);
        //printf("user data is %d\n", in_param);
        if (len <0 && errno != EINTR) {
            printf("read error %d (%d)\n", ec->fd, errno);
            return -1;
        }
        else if(len == 0){
            break;
        }

        if(len<remaning_length){
            readLen = len;
            break;
        }

        remaning_length-=len;
        readLen += len;
        ptr += (int)len;
    }
    printf("sock %d, len %d: %s\n", ec->fd, (int)readLen, buf);
    write(ec->fd, buf, readLen);

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
