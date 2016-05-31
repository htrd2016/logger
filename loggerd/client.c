#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client.h"
#include "types.h"

void init_client(Client *pClient) {
  pClient->pBlock = NULL;
  pClient->nPort = -1;
  memset(pClient->szIP, 0, sizeof(pClient->szIP));
  pClient->nfd = 0;
  pClient->free = true;
}

Client *get_one_free_client(Client clients[], int nCount) {
  int i = 0;
  for (i = 0; i < nCount; i++) {
    Client *pClient = &clients[i];
    if (pClient->free == true) {
      return pClient;
    }
  }
  return NULL;
}

void init_clients() {
  uint32 i = 0;

  clients = calloc(1, configData.block_amount * sizeof(Client));

  for (i = 0; i < configData.block_amount; i++) {
    init_client(&clients[i]);
  }
}

void close_clients() {
  uint32 i = 0;
  for (i = 0; i < configData.block_amount; i++) {
    if (clients[i].nfd > 0 && clients[i].free == false) {
      close(clients[i].nfd);
    }
  }
}
