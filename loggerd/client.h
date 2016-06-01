#ifndef ___CLIENT_H_INCLUDE___
#define ___CLIENT_H_INCLUDE___

#include <netinet/in.h>
#include "utils.h"
#include "types.h"

typedef struct{
    Block *pBlock;
    int nPort;
    char szIP[32];
}ClientData;

typedef struct
{
    void* pData;
    struct sockaddr_in cliaddr;
    int fd;
    bool free;
}EpollClient;

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

extern EpollClient *clients;

int init_epoll_clients();
void close_epoll_clients();

void release_client_datas();
int init_client_datas();

EpollClient* get_a_free_epoll_client();
//ClientData *get_one_free_client_data(ClientData clients[], int nCount);

#ifdef __cplusplus
}
#endif //__cplusplus



#endif //___CLIENT_H_INCLUDE___
