#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "utils.h"

static void init_indicator();
static int create_recv_buffer_mem();
static int create_indicator_mem();
static void init_recv_buffer(RecvBuffer *pRecvBuffer, uint32 block_index,
                             uint32 buf_index);
static void init_thread_data(ThreadData *pThreadData);

#define release_memory(ptr)                                                    \
  if (ptr) {                                                                   \
    free(ptr);                                                                 \
    ptr = 0;                                                                   \
  }

int init_mem() {
  if (0 != create_recv_buffer_mem()) {
    return -1;
  }

  if (0 != create_indicator_mem()) {
    release_memory(configData.recv_data_memory);
    return -1;
  }

  init_indicator();
  return 0;
}

void release_mem() {
  release_memory(configData.indicator_memory);
  release_memory(configData.recv_data_memory);
  memset(&configData, 0, sizeof(configData));
}

static int create_indicator_mem() {
  configData.indicator_memory = (uchar *)calloc(
      1,
      (sizeof(Block) + configData.buffer_amount_in_block * sizeof(RecvBuffer)) *
              configData.block_amount +
          configData.thread_amount * sizeof(ThreadData));
  if (configData.indicator_memory) {
    configData.pBlock = (Block *)configData.indicator_memory;
    configData.pThreadData = (ThreadData *)(configData.indicator_memory +
                                            (sizeof(Block) +
                                             configData.buffer_amount_in_block *
                                                 sizeof(RecvBuffer)) *
                                                configData.block_amount);
    return 0;
  } else
    return (-1);
}

static int create_recv_buffer_mem() {

  configData.recv_data_memory = (uchar *)calloc(
      1, configData.block_amount * configData.buffer_amount_in_block *
             configData.size_of_buffer);
  return (configData.recv_data_memory == 0) ? (-1) : 0;
}

static void init_recv_buffer(RecvBuffer *pRecvBuffer, uint32 block_index,
                             uint32 buf_index) {
  pRecvBuffer->data_start_ptr = pRecvBuffer->buf_start =
      pRecvBuffer->data_end_ptr =
          configData.recv_data_memory +
          (block_index * configData.buffer_amount_in_block + buf_index) *
              configData.size_of_buffer;
  pRecvBuffer->buf_end = pRecvBuffer->buf_start + configData.size_of_buffer - 1;
  // pRecvBuffer->to_write_ptr = pRecvBuffer->data_start_ptr;
  pRecvBuffer->free = true;
}

static void init_thread_data(ThreadData *pThreadData) {
  pThreadData->block = NULL;
  //    pThreadData->end_data_pt = NULL;
  //    pThreadData->start_data_pt = NULL;
  pThreadData->recv_buffer = NULL;
  pThreadData->free = true;
  pThreadData->have_data = false;
}

static void init_indicator() {
  uint32 i = 0;
  uint32 j = 0;

  for (i = 0; i < configData.block_amount; i++) {
    configData.pBlock[i].free = true;
    configData.pBlock[i].bufIndexToWrite = 0;
    configData.pBlock[i].buffer_amount = configData.buffer_amount_in_block;
    configData.pBlock[i].recvBufs =
        (RecvBuffer *)(configData.indicator_memory +
                       configData.block_amount * sizeof(Block) +
                       configData.buffer_amount_in_block * sizeof(RecvBuffer) * i);
    for (j = 0; j < configData.buffer_amount_in_block; j++) // init buffers
    {
      init_recv_buffer(configData.pBlock[i].recvBufs + j, i, j);
    }
    configData.pBlock[i].pHeadPtr =
        (uchar *)(configData.pBlock[i].recvBufs->buf_start);
    configData.pBlock[i].pTailPtr =
        configData.pBlock[i].pHeadPtr +
        configData.size_of_buffer * configData.buffer_amount_in_block - 1;
  }

  for (i = 0; i < configData.thread_amount; i++) // init threaddatas
  {
    init_thread_data(configData.pThreadData + i);
  }
}

ThreadData *get_free_thread_data() {
  uint32 i = 0;
  for (i = 0; i < configData.thread_amount; i++) {
    if (configData.pThreadData[i].free == true) {
      printf("get_one_free_thread_data index =%d\n", i);
      return configData.pThreadData + i;
    }
  }
  return NULL;
}

void reset_block(Block *pBlock) {
  uint32 i = 0;
  pBlock->free = true;
  pBlock->bufIndexToWrite = 0;
  for (; i < configData.buffer_amount_in_block; i++) {
    pBlock->recvBufs[i].data_end_ptr = pBlock->recvBufs[i].data_start_ptr =
        pBlock->recvBufs[i].buf_start;
    pBlock->recvBufs[i].free = true;
  }
}

Block *get_free_block() {
  uint32 i = 0;
  for (i = 0; i < configData.block_amount; i++) {
    if (configData.pBlock[i].free == true) {
      reset_block(configData.pBlock + i);
      return (configData.pBlock + i);
    }
  }
  return NULL;
}

RecvBuffer *get_next_free_recv_buffer(Block *pBlock) {
  if (pBlock->recvBufs[pBlock->bufIndexToWrite].free)
    return pBlock->recvBufs + pBlock->bufIndexToWrite;
  else
    return (0);
}
