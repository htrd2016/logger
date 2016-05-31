#ifndef ___CLIENT_H_INCLUDE___
#define ___CLIENT_H_INCLUDE___

#include "utils.h"
#include "types.h"

typedef struct structClient
{
    Block *pBlock;
	int nPort;
	char szIP[32];
    int nfd;
    bool free;
}Client; 

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

extern Client *clients;

void init_clients();
void close_clients();

Client *get_one_free_client(Client clients[], int nCount);

#ifdef __cplusplus
}
#endif //__cplusplus



#endif //___CLIENT_H_INCLUDE___
