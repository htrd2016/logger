#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>



typedef int (*accept_callback) (int connfd, struct sockaddr_in* cliaddr, void** out_data);
typedef int (*read_callback) (int connfd, struct sockaddr_in* cliaddr, void* in_param);


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

extern int server(accept_callback accept_fun, read_callback read_fun);

#ifdef __cplusplus
}
#endif //__cplusplus


#endif // SERVER_H
