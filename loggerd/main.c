#include <stdio.h>
#include <stdlib.h>

#include "mylog.h"
#include "parserthread.h"
#include "server.h"
#include "types.h"

ConfData configData;
extern int logger_accept(int connfd, struct sockaddr_in* cliaddr, void* in_param, void** out_data);
extern int handle(int connfd, struct sockaddr_in* cliaddr, void* in_param, void** out_data);

static int get_args(int argc, char *argv[]) {
  if (argc != 6) {
    printf("path <local port> <thread number> <buffer size(bytes)> <buffer "
           "number in block> <block number>\n");
    return -1;
  }

  configData.local_port = atoi(argv[1]);
  configData.thread_amount = atoi(argv[2]);
  configData.size_of_buffer = (size_t)atoi(argv[3]);
  configData.buffer_amount_in_block = atoi(argv[4]);
  configData.block_amount = atoi(argv[5]);
  return 0;
}

int main(int argc, char *argv[]) {
  int result = 0;
  set_debug_flag(L_DBG);

  configData.stop = get_args(argc, argv);
  if (configData.stop) {
    return -1;
  }

  if (0 != init_mem()) {
    configData.stop = 1;
    mylog(configData.logfile, L_ERR, "init mem failed!!!");
    printf("init mem failed\n");
  }

  if (!configData.stop) {
    if (0 != create_parse_threads()) {
      printf("error create_parse_threads failed!!");
      configData.stop = 1;
    }
  }

  result = server(logger_accept, handle);

  sleep(10);
  release_mem();

  return result;
}
