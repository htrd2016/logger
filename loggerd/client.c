#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client.h"
#include "types.h"

//static void init_client(Client *pClient) {
//  pClient->pBlock = NULL;
//  pClient->nPort = -1;
//  memset(pClient->szIP, 0, sizeof(pClient->szIP));
//  pClient->nfd = 0;
//  pClient->free = true;
//}

static void *client_datas_memory = 0;
static void init_epoll_client(EpollClient* pEpollClient)
{
    pEpollClient->free = true;
    pEpollClient->fd = -1;
    pEpollClient->pData = NULL;
    memset(&pEpollClient->cliaddr, 0, sizeof(pEpollClient->cliaddr));
}

void init_epoll_clients() {
  uint32 i = 0;
  clients = calloc(1, configData.block_amount * sizeof(EpollClient));

  for (i = 0; i < configData.block_amount; i++) {
    init_epoll_client(&clients[i]);
  }
}

void init_client_datas(){
    client_datas_memory = calloc(1, configData.block_amount * sizeof(ClientData));
    uint32 i = 0;
    for(i=0;i<configData.block_amount;i++){
        ClientData *client_data = client_datas_memory+i*sizeof(ClientData);
        client_data->nPort = -1;
        memset(client_data->szIP, 0, 32);
        clients[i].pData = client_data;
    }
}

void release_client_datas(){
    if(client_datas_memory != 0){
        free(client_datas_memory);
        client_datas_memory = 0;
    }
}

EpollClient* get_a_free_epoll_client(){
    uint32 i = 0;
    for (i = 0; i < configData.block_amount; i++) {
      if(clients[i].free == true){
          return &clients[i];
      }
    }
    return NULL;
}

void close_epoll_clients() {
  uint32 i = 0;
  for (i = 0; i < configData.block_amount; i++) {
    if (clients[i].fd > 0 && clients[i].free == false) {
      close(clients[i].fd);
    }
  }
}

//ClientData *get_one_free_client_data(Client clients[], int nCount) {
//  int i = 0;
//  for (i = 0; i < nCount; i++) {
//    Client *pClient = &clients[i];
//    if (pClient->free == true) {
//      return pClient;
//    }
//  }
//  return NULL;
//}
