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
#include "memtypes.h"
#include "mylog.h"
#include "utils.h"
#include "epollclient.h"

static void *pasreProc(void *);

//May 06 19:51:20 hitrade1 tkernel 1[1667]: XTPPub SubjectID=0X1001, Flow Size=21798, PublishPort: 21798
void parseLine(const uchar *line_start_ptr, const uchar *line_end_ptr)
{
    uchar t[16];
    char* start = NULL;
    char info[30];
    char detail[1024];

    if(line_end_ptr-line_start_ptr<41)
    {
        char buf[1024];
        memcpy(buf, line_start_ptr, line_end_ptr-line_start_ptr+1);
        buf[line_end_ptr-line_start_ptr+1] = '\0';
        printf("------(%s)", buf);
        printf("parse line error\n");
        mylog(configData.logfile, L_ERR, "parse line [%s] error,len=%d", buf);

        return;
    }

    start = (char*)line_start_ptr;

    memcpy(t ,start, 15);
    t[15] = '\0';
    printf("time=%s ", t);

    start = start+16;

    memcpy(info, start, 25);
    info[25] = '\0';
    printf("%s ", info);

    start = start+26;
    memcpy(detail, start, (char*)line_end_ptr-(char*)start+1);
    detail[(char*)line_end_ptr-(char*)start+1] = '\0';
    printf("%s\n", detail);
}

void *pasreProc(void *p) {
    ThreadData *pThreadData = NULL;
    static int threadIndex = 0;
    threadIndex++;
    int nThreadID = threadIndex;
    pThreadData = (ThreadData *)p;
    uchar *buffer_start = NULL;
    uchar *line_start = NULL;
    int is_full_line = false;

//  char fileName[120];
//  sprintf(fileName, "./log%d.txt", threadIndex);
//  FILE *file = fopen(fileName, "a+");

  while (!configData.stop) {
    if (pThreadData->have_data == true) {
      pThreadData->free = false;
      buffer_start = pThreadData->recv_buffer->data_start_ptr;
      int remaning_length = pThreadData->recv_buffer->data_end_ptr -
                         pThreadData->recv_buffer->data_start_ptr + 1;

      while (true) {
        int nLength = get_line(buffer_start, remaning_length, &line_start,
                                    &remaning_length, &is_full_line);
        if (nLength > 0) {
            parseLine(line_start, line_start+nLength-1);
        }
        else if(nLength<0)
        {
            mylog(configData.logfile, L_ERR, "get_line return %d", nLength);
            configData.stop = 1;
            break;
        }
        else if (remaning_length == 0) {
          pThreadData->recv_buffer->data_end_ptr =
              pThreadData->recv_buffer->data_start_ptr;
          pThreadData->recv_buffer->free = true;
          pThreadData->free = true;
          pThreadData->have_data = false;
          printf("thread %d pasre over\n", nThreadID);
          mylog(configData.logfile, L_INFO, "thread %d pasre over", nThreadID);
          break;
        }
        buffer_start = line_start + nLength;
      }
    } else {
      // printf("free!!!\n");
      usleep(1);
    }
  }
  //fclose(file);
  printf("thread exit ... \n");
  mylog(configData.logfile, L_INFO, "parse thread exit...");
  pthread_detach(pthread_self());
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

int logger_accept(int connfd, struct sockaddr_in *cliaddr, void **in_data) {
  ClientData *pClientData = 0;
  Block *pBlock = 0;
  EpollClient *pClient = (EpollClient*)(*in_data);

  if (pClient == NULL) {
    configData.stop = 1;
    mylog(configData.logfile, L_ERR, "no more free client to find max=%d",
          configData.block_amount);
    return -1;
  }

  pClientData = (ClientData*)pClient->pData;
  strcpy(pClientData->szIP, inet_ntoa(cliaddr->sin_addr));
  pClientData->nPort = cliaddr->sin_port;
  pClient->fd = connfd;
  pClient->free = false;

  pBlock = get_free_block();
  if (pBlock == NULL) {
    configData.stop = 1;
    mylog(configData.logfile, L_ERR, "no more free b block find max=%d",
          configData.block_amount);
    return -2;
  }
  pBlock->free = false;
  pClientData->pBlock = pBlock;

  return (0);
}

ssize_t read_socket_to_recv_buffer(EpollClient *in_client,
                                   ThreadData *out_thread_data) {

  ClientData *pClientData;
  Block *block;
  ssize_t remaning_length;
  RecvBuffer *pRecvBuffer;
  int connfd;
  uchar *write_ptr;
  uchar *write_prt_tmp;
  size_t total_read_len = 0;
  ssize_t nread;

  uchar *tail_line_start = NULL;
  uchar *tail_line_end = NULL;
  int have_full_line = 0;
  bool have_half_line;
  size_t copy_len;

  RecvBuffer *second_recv_buffer;

  out_thread_data->free = false;
  connfd = in_client->fd;

  pClientData = (ClientData*)in_client->pData;
  block = pClientData->pBlock;

  pRecvBuffer = get_next_free_recv_buffer(block);
  if (pRecvBuffer == NULL) {
    configData.stop = 1;
    mylog(configData.logfile, L_ERR, "get free recvbuffer failed!!");
    printf("get free recvbuffer failed!!");
    out_thread_data->free = true;
    return -1;
  }

  remaning_length = configData.size_of_buffer;
  if (pRecvBuffer->data_end_ptr != pRecvBuffer->buf_start) {
    remaning_length =
        remaning_length - (pRecvBuffer->data_end_ptr - pRecvBuffer->buf_start + 1);
  }

  write_ptr = (pRecvBuffer->data_end_ptr == pRecvBuffer->buf_start)
                         ? (pRecvBuffer->buf_start)
                         : (pRecvBuffer->data_end_ptr + 1);
  write_prt_tmp = write_ptr;
  while(remaning_length>0)
  {
      nread = read(connfd, write_prt_tmp, remaning_length); //读取客户端socket流

      if (nread == 0) {
          //EOF ，说明客户端发来了FIN；
          if(total_read_len == 0){
              return 0;
          }
          break;
      }

      if (nread < 0 && errno != EINTR) {//read error
          perror("read error");
          reset_block(block);
          out_thread_data->free = true;
          return -1;
      }

      if(nread<remaning_length){
          total_read_len = nread;
          break;
      }

      write_prt_tmp += (int)nread;
      remaning_length -= nread;
      total_read_len += nread;
  }

  have_half_line = get_end_half_line(pRecvBuffer->buf_start, write_ptr + total_read_len - 1,
                        &tail_line_start, &tail_line_end, &have_full_line);

  // 1.one harf line "111111"
  if (have_full_line == 0 && have_half_line == true) {
    pRecvBuffer->data_end_ptr = write_ptr + total_read_len - 1;
    out_thread_data->free = true;
    return 0;
  }
  // 2.one or more full lines "111\r\n2222\r\n"
  else if (have_full_line == 1 && have_half_line == false) {
    block->bufIndexToWrite++;
    if (block->bufIndexToWrite >= configData.buffer_amount_in_block) {
      block->bufIndexToWrite = 0;
    }
    pRecvBuffer->data_end_ptr = write_ptr + total_read_len - 1;
    out_thread_data->recv_buffer = pRecvBuffer;
    out_thread_data->have_data = true;
  }
  // 3.one or more full lines and harf line "111\n22222"
  else {
    block->bufIndexToWrite++;
    if (block->bufIndexToWrite >= configData.buffer_amount_in_block) {
      block->bufIndexToWrite = 0;
    }

    second_recv_buffer = get_next_free_recv_buffer(block);
    if (second_recv_buffer == NULL) {
      printf("no more free recv buffer to get!!");
      mylog(configData.logfile, L_ERR, "no more free recv buffer to get!!");
      configData.stop = 1;
      out_thread_data->free = true;
      return -1;
    }
    copy_len = tail_line_end - tail_line_start + 1;
    memcpy(second_recv_buffer->data_start_ptr, tail_line_start, copy_len);
    second_recv_buffer->data_end_ptr =
        second_recv_buffer->data_end_ptr + copy_len - 1;
    memset(tail_line_start, 0, copy_len);
    *tail_line_start = '\n';
    pRecvBuffer->data_end_ptr = pRecvBuffer->data_end_ptr + total_read_len - copy_len;
    out_thread_data->recv_buffer = pRecvBuffer;
    out_thread_data->have_data = true;
  }
  return total_read_len;
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
