#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

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

#include "parserthread.h"
#include "types.h"
#include "mylog.h"
#include "utils.h"
#include "client.h"

static void *pasreProc(void *);


void *pasreProc(void *p) {
  static int threadIndex = 0;
  threadIndex++;
  int nThreadID = threadIndex;
  ThreadData *pThreadData = (ThreadData *)p;
  char fileName[120];
  sprintf(fileName, "./log%d.txt", threadIndex);
  FILE *file = fopen(fileName, "a+");
  while (!configData.stop) {
    if (pThreadData->have_data == true) {
      pThreadData->free = false;
      uchar *start = pThreadData->recv_buffer->data_start_ptr;
      int src_left_len = pThreadData->recv_buffer->data_end_ptr -
                         pThreadData->recv_buffer->data_start_ptr + 1;

      uchar *line_start = NULL;
      int is_full_line = false;
      while (true) {
        int nLength = get_line(start, src_left_len, &line_start,
                                    &src_left_len, &is_full_line);
        if (nLength > 0) {
          char buf[1024];
          memcpy(buf, line_start, nLength);
          buf[nLength] = '\0';
          strcat(buf, "\n");
          static int lineId = 1;
          fwrite(buf, strlen(buf), 1, file);
          fflush(file);
          lineId++;
        }

        if (src_left_len == 0) {
          pThreadData->recv_buffer->data_end_ptr =
              pThreadData->recv_buffer->data_start_ptr;
          pThreadData->recv_buffer->free = true;
          pThreadData->free = true;
          pThreadData->have_data = false;
          printf("thread %d pasre over\n", nThreadID);
          mylog(configData.logfile, L_INFO, "thread %d pasre over", nThreadID);
          break;
        }
        start = line_start + nLength;
      }
    } else {
      // printf("free!!!\n");
      usleep(1);
    }
  }
  fclose(file);
  mylog(configData.logfile, L_INFO, "parse thread exit...");
  return 0;
}

int create_parse_threads() {
  uint32 i;

  for (i = 0; i < configData.thread_amount; i++) {
    pthread_t id;
    if (0 != pthread_create(&id, NULL, pasreProc, configData.pThreadData + i))
      return -1;
  }
  return 0;
}

int logger_accept(int connfd, struct sockaddr_in *cliaddr, void **out_data) {
  EpollClient *pClient = get_a_free_epoll_client(clients, configData.block_amount);
  if (pClient == NULL) {
    configData.stop = 1;
    mylog(configData.logfile, L_ERR, "no more free client to find max=%d",
          configData.block_amount);
    return -1;
  }

  ClientData *pClientData = (ClientData*)pClient->pData;
  strcpy(pClientData->szIP, inet_ntoa(cliaddr->sin_addr));
  pClientData->nPort = cliaddr->sin_port;
  pClient->fd = connfd;
  pClient->free = false;

  Block *pBlock = get_free_block();
  if (pBlock == NULL) {
    configData.stop = 1;
    mylog(configData.logfile, L_ERR, "no more free b block find max=%d",
          configData.block_amount);
    return -2;
  }
  pBlock->free = false;
  pClientData->pBlock = pBlock;

  *out_data = pClient;

  return (0);
}



ssize_t read_socket_to_recv_buffer(EpollClient *in_client,
                                   ThreadData *out_thread_data) {
  out_thread_data->free = false;
  int connfd = in_client->fd;
  ClientData *pClientData = (ClientData*)in_client->pData;
  Block *block = pClientData->pBlock;
  RecvBuffer *pRecvBuffer = get_next_free_recv_buffer(block);
  if (pRecvBuffer == NULL) {
    configData.stop = 1;
    mylog(configData.logfile, L_ERR, "get free recvbuffer failed!!");
    printf("get free recvbuffer failed!!");
    out_thread_data->free = true;
    return -1;
  }

  ssize_t nLeftLength = configData.size_of_buffer - 1;
  if (pRecvBuffer->data_end_ptr != pRecvBuffer->buf_start) {
    nLeftLength =
        nLeftLength - (pRecvBuffer->data_end_ptr - pRecvBuffer->buf_start + 1);
  }

  *pRecvBuffer->buf_end = '\0';
  uchar *write_ptr = (pRecvBuffer->data_end_ptr == pRecvBuffer->buf_start)
                         ? (pRecvBuffer->buf_start)
                         : (pRecvBuffer->data_end_ptr + 1);
  ssize_t nread = read(connfd, write_ptr, nLeftLength); //读取客户端socket流
  if (nread == 0) {
    printf("client close the connection\n");
    mylog(configData.logfile, L_INFO, "client close the connection: %s %d",
          pClientData->szIP, pClientData->nPort);
    in_client->free = true;
    reset_block(block);
    out_thread_data->free = true;

    return -1;
  }

  if (nread < 0 && errno != EINTR) {
    perror("read error");
    close(connfd);
    in_client->free = true;
    reset_block(block);
    out_thread_data->free = true;
    return -1;
  }

  uchar *line_start = NULL;
  uchar *line_end = NULL;
  int have_multi_line = 0;
  bool have_half_line =
      get_end_half_line(pRecvBuffer->buf_start, write_ptr + nread - 1,
                        &line_start, &line_end, &have_multi_line);

  // 1.one harf line "111111"
  if (have_multi_line == 0 && have_half_line == true) {
    pRecvBuffer->data_end_ptr = write_ptr + nread - 1;
    out_thread_data->free = true;
    return 0;
  }
  // 2.one or more full lines "111\r\n2222\r\n"
  else if (have_multi_line == 1 && have_half_line == false) {
    block->bufIndexToWrite++;
    if (block->bufIndexToWrite >= configData.buffer_amount_in_block) {
      block->bufIndexToWrite = 0;
    }
    pRecvBuffer->data_end_ptr = write_ptr + nread - 1;
    out_thread_data->recv_buffer = pRecvBuffer;
    out_thread_data->have_data = true;
  }
  // 3.one or more full lines and harf line "111\n22222"
  else {
    block->bufIndexToWrite++;
    if (block->bufIndexToWrite >= configData.buffer_amount_in_block) {
      block->bufIndexToWrite = 0;
    }

    RecvBuffer *second_recv_buffer = get_next_free_recv_buffer(block);
    if (second_recv_buffer == NULL) {
      printf("no more free recv buffer to get!!");
      mylog(configData.logfile, L_ERR, "no more free recv buffer to get!!");
      configData.stop = 1;
      out_thread_data->free = true;
      return -1;
    }
    size_t copy_len = line_end - line_start + 1;
    memcpy(second_recv_buffer->data_start_ptr, line_start, copy_len);
    second_recv_buffer->data_end_ptr =
        second_recv_buffer->data_end_ptr + copy_len - 1;
    *line_start = '\0';
    pRecvBuffer->data_end_ptr = pRecvBuffer->data_end_ptr + nread - copy_len;
    out_thread_data->recv_buffer = pRecvBuffer;
    out_thread_data->have_data = true;
  }
  return nread;
}

int handle(void *in_param) {
  EpollClient * pClient = (EpollClient *)in_param;
  ThreadData *pThreadData = get_free_thread_data();
  if (pThreadData == NULL) {
    configData.stop = 1;
    mylog(configData.logfile, L_ERR, "can not find free thread!!\n");
    return -1;
  }

  return read_socket_to_recv_buffer(pClient, pThreadData);
}
