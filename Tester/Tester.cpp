#include "gtest/gtest.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "utils.h"
#include "types.h"
#include "mylog.h"

ConfData configData;

TEST(TestTypedefs, TestTypedefs) {
  configData.block_amount = 10;
  configData.buffer_amount_in_block = 3;
  configData.size_of_buffer = 1024;
  configData.thread_amount = 3;

  int nRet = init_mem();
  EXPECT_EQ(nRet, 0);

  uchar *indicator = configData.indicator_memory;
  uchar *recv_buf = configData.recv_data_memory;

  EXPECT_EQ(reinterpret_cast<void *>(indicator),
            reinterpret_cast<void *>(configData.pBlock));
  EXPECT_EQ(reinterpret_cast<void *>(
                indicator + configData.block_amount * sizeof(Block) +
                configData.block_amount * configData.buffer_amount_in_block *
                    sizeof(RecvBuffer)),
            reinterpret_cast<void *>(configData.pThreadData));
  EXPECT_EQ(configData.pBlock[0].recvBufs[0].buf_start, recv_buf);

  for (uint32 block_pos = 0; block_pos < configData.block_amount; ++block_pos) {
    Block *block = &configData.pBlock[block_pos];
    EXPECT_EQ(block, configData.pBlock + block_pos);
    EXPECT_EQ(block->buffer_amount, configData.buffer_amount_in_block);
    EXPECT_EQ(block->bufIndexToWrite, 0);
    EXPECT_EQ(block->free, 1);
    EXPECT_EQ(block->pHeadPtr, block->recvBufs->buf_start);
    EXPECT_EQ(block->pTailPtr - block->pHeadPtr + 1,
              configData.buffer_amount_in_block * configData.size_of_buffer);
    EXPECT_EQ(reinterpret_cast<uchar *>(block->recvBufs) -
                  reinterpret_cast<uchar *>(block),
              (configData.block_amount - block_pos) * sizeof(Block) +
                  block_pos *
                      (configData.buffer_amount_in_block * sizeof(RecvBuffer)));

    for (uint32 recbuf_pos = 0; recbuf_pos < configData.buffer_amount_in_block;
         ++recbuf_pos) {
      RecvBuffer *recvbuf = &block->recvBufs[recbuf_pos];
      EXPECT_EQ(recvbuf->buf_start, recv_buf +
                                        block_pos *
                                            configData.buffer_amount_in_block *
                                            configData.size_of_buffer +
                                        recbuf_pos * configData.size_of_buffer);
      EXPECT_EQ(recvbuf->buf_end - recvbuf->buf_start + 1,
                configData.size_of_buffer);
      EXPECT_EQ(recvbuf->buf_start, recvbuf->data_end_ptr);
      EXPECT_EQ(recvbuf->buf_start, recvbuf->data_start_ptr);
      EXPECT_EQ(recvbuf->free, true);
    }
  }

  for (uint32 thread_pos = 0; thread_pos < configData.thread_amount;
       ++thread_pos) {
    ThreadData *thread = &configData.pThreadData[thread_pos];
    EXPECT_EQ(thread->free, true);
    EXPECT_EQ(thread->have_data, false);
    EXPECT_EQ(reinterpret_cast<void *>(thread->recv_buffer),
              reinterpret_cast<void *>(0));
    EXPECT_EQ(reinterpret_cast<void *>(thread->block),
              reinterpret_cast<void *>(0));
  }

  release_mem();

  EXPECT_EQ(configData.indicator_memory, reinterpret_cast<uchar *>(0));
  EXPECT_EQ(configData.recv_data_memory, reinterpret_cast<uchar *>(0));
  uchar *zero_mem = (uchar *)calloc(1, sizeof(configData));
  EXPECT_EQ(memcmp(&configData, zero_mem, sizeof(configData)), 0);
  free(zero_mem);

  configData.block_amount = 20;
  configData.buffer_amount_in_block = 5;
  configData.size_of_buffer = 1024 * 1024;
  configData.thread_amount = 10;

  nRet = init_mem();
  EXPECT_EQ(nRet, 0);

  indicator = configData.indicator_memory;
  recv_buf = configData.recv_data_memory;

  EXPECT_EQ(reinterpret_cast<void *>(indicator),
            reinterpret_cast<void *>(configData.pBlock));
  EXPECT_EQ(reinterpret_cast<void *>(
                indicator + configData.block_amount * sizeof(Block) +
                configData.block_amount * configData.buffer_amount_in_block *
                    sizeof(RecvBuffer)),
            reinterpret_cast<void *>(configData.pThreadData));
  EXPECT_EQ(configData.pBlock[0].recvBufs[0].buf_start, recv_buf);

  for (uint32 block_pos = 0; block_pos < configData.block_amount; ++block_pos) {
    Block *block = &configData.pBlock[block_pos];
    EXPECT_EQ(block, configData.pBlock + block_pos);
    EXPECT_EQ(block->buffer_amount, configData.buffer_amount_in_block);
    EXPECT_EQ(block->bufIndexToWrite, 0);
    EXPECT_EQ(block->free, 1);
    EXPECT_EQ(block->pHeadPtr, block->recvBufs->buf_start);
    EXPECT_EQ(block->pTailPtr - block->pHeadPtr + 1,
              configData.buffer_amount_in_block * configData.size_of_buffer);
    EXPECT_EQ(reinterpret_cast<uchar *>(block->recvBufs) -
                  reinterpret_cast<uchar *>(block),
              (configData.block_amount - block_pos) * sizeof(Block) +
                  block_pos *
                      (configData.buffer_amount_in_block * sizeof(RecvBuffer)));

    for (uint32 recbuf_pos = 0; recbuf_pos < configData.buffer_amount_in_block;
         ++recbuf_pos) {
      RecvBuffer *recvbuf = &block->recvBufs[recbuf_pos];
      EXPECT_EQ(recvbuf->buf_start, recv_buf +
                                        block_pos *
                                            configData.buffer_amount_in_block *
                                            configData.size_of_buffer +
                                        recbuf_pos * configData.size_of_buffer);
      EXPECT_EQ(recvbuf->buf_end - recvbuf->buf_start + 1,
                configData.size_of_buffer);
      EXPECT_EQ(recvbuf->buf_start, recvbuf->data_end_ptr);
      EXPECT_EQ(recvbuf->buf_start, recvbuf->data_start_ptr);
      EXPECT_EQ(recvbuf->free, true);
    }
  }

  for (uint32 thread_pos = 0; thread_pos < configData.thread_amount;
       ++thread_pos) {
    ThreadData *thread = &configData.pThreadData[thread_pos];
    EXPECT_EQ(thread->free, true);
    EXPECT_EQ(thread->have_data, false);
    EXPECT_EQ(reinterpret_cast<void *>(thread->recv_buffer),
              reinterpret_cast<void *>(0));
    EXPECT_EQ(reinterpret_cast<void *>(thread->block),
              reinterpret_cast<void *>(0));
  }

  release_mem();

  EXPECT_EQ(configData.indicator_memory, reinterpret_cast<uchar *>(0));
  EXPECT_EQ(configData.recv_data_memory, reinterpret_cast<uchar *>(0));
  zero_mem = (uchar *)calloc(1, sizeof(configData));
  EXPECT_EQ(memcmp(&configData, zero_mem, sizeof(configData)), 0);
  free(zero_mem);
}

TEST(TestgetEndHalfLine, getEndHalfLine0) {
  uchar szData[512] = "log0";
  uchar *pLineStart = NULL;
  uchar *pLineEnd = NULL;
  int haveMultiLine = 0;
  bool bRet = get_end_half_line(szData, szData + strlen((char *)szData),
                                &pLineStart, &pLineEnd, &haveMultiLine);
  EXPECT_EQ(bRet, true);
  EXPECT_EQ(haveMultiLine, false);
  EXPECT_EQ(pLineStart, szData);
  EXPECT_EQ(strcmp((char*)pLineStart, "log0"), 0);
}

TEST(TestgetEndHalfLine, getEndHalfLine1) {
  int haveMultiLine = 0;
  uchar szData[512] = "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog1";
  uchar *pLineStart = NULL;
  uchar *pLineEnd = NULL;
  bool bRet = get_end_half_line(szData, szData + strlen((char *)szData),
                                &pLineStart, &pLineEnd, &haveMultiLine);
  EXPECT_EQ(bRet, true);
  EXPECT_EQ(haveMultiLine, true);
  EXPECT_EQ(pLineStart, szData + strlen((char*)szData) - strlen("log1"));
  EXPECT_EQ(strcmp((char*)pLineStart, "log1"), 0);
}


TEST(TestgetEndHalfLine, getEndHalfLine2) {
  uchar szData[512] = "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\n";
  uchar *pLineStart = NULL;
  uchar *pLineEnd = NULL;
  int haveMultiLine = 0;
  bool bRet = get_end_half_line(szData, szData + strlen((char *)szData),
                                &pLineStart, &pLineEnd, &haveMultiLine);
  EXPECT_EQ(bRet, false);
  EXPECT_EQ(haveMultiLine, true);
  EXPECT_EQ(pLineStart, szData + strlen((char*)szData));
  EXPECT_EQ(strcmp((char*)pLineStart, ""), 0);
}

TEST(readNextLine, readNextLine0) {
  int isFullLine = 0;
  char szData[1024] = "May\n 06 19:39:58 hitrade1\r\n Receive Nak";

  char *pLineStart = NULL;
  char *pLineEnd = NULL;
  int nLen = read_next_line(szData, &pLineStart, &pLineEnd, &isFullLine);
  EXPECT_STREQ(pLineStart, "May");
  EXPECT_EQ(pLineStart, szData);
  EXPECT_EQ(pLineEnd, szData + 3);
  EXPECT_EQ(nLen, 3);
  EXPECT_EQ(strlen(pLineStart), 3);
  EXPECT_EQ(isFullLine, 1);

  nLen = read_next_line(pLineEnd + 1, &pLineStart, &pLineEnd, &isFullLine);
  EXPECT_STREQ(pLineStart, " 06 19:39:58 hitrade1");
  EXPECT_EQ(pLineStart, szData + 3 + 1);
  EXPECT_EQ(pLineEnd, pLineStart + 21);
  EXPECT_EQ(nLen, 21);
  EXPECT_EQ(strlen(pLineStart), 21);
  EXPECT_EQ(isFullLine, 1);

  nLen = read_next_line(pLineEnd + 1, &pLineStart, &pLineEnd, &isFullLine);
  EXPECT_STREQ(pLineStart, " Receive Nak");
  EXPECT_EQ(pLineStart, szData + 3 + 1 + 21 + 2);
  EXPECT_EQ(pLineEnd, pLineStart + 12);
  EXPECT_EQ(nLen, 12);
  EXPECT_EQ(strlen(pLineStart), 12);
  EXPECT_EQ(isFullLine, 0);

  nLen = read_next_line(pLineEnd + 1, &pLineStart, &pLineEnd, &isFullLine);
  EXPECT_STREQ(pLineStart, "");
  EXPECT_EQ(pLineEnd, pLineStart);
  EXPECT_EQ(nLen, 0);
  EXPECT_EQ(strlen(pLineStart), 0);
  EXPECT_EQ(isFullLine, 0);
}

TEST(get_next_line, get_next_line) {
  int nFullLine = 0;
  uchar szData[1024] = "May\n 06 19:39:58 hitrade1\r\n Receive Nak";

  uchar *pLineStart = NULL;
  int srcLength = strlen((char *)szData);
  int nLen =
      get_next_line(szData, srcLength, &pLineStart, &srcLength, &nFullLine);
  EXPECT_EQ(pLineStart, szData);
  EXPECT_EQ(nLen, 3);
  EXPECT_EQ(srcLength, 36);
  EXPECT_EQ(nFullLine, 1);

  pLineStart = pLineStart + nLen;
  nLen =
      get_next_line(pLineStart, srcLength, &pLineStart, &srcLength, &nFullLine);
  EXPECT_STREQ((char *)pLineStart, (char *)szData + 4);
  EXPECT_EQ(nLen, 21);
  EXPECT_EQ(srcLength, 14);
  EXPECT_EQ(nFullLine, 1);

  pLineStart = pLineStart + nLen;
  nLen =
      get_next_line(pLineStart, srcLength, &pLineStart, &srcLength, &nFullLine);
  EXPECT_STREQ((char *)pLineStart, " Receive Nak");
  EXPECT_EQ(nLen, 12);
  EXPECT_EQ(srcLength, 0);
  EXPECT_EQ(nFullLine, 0);
}

TEST(readNextLine, readNextLine1) {
  int nFullLine = 0;
  char szData[1024] = "May\n 06 19:39:58 hitrade1\r\n";

  char *pLineStart = NULL;
  char *pLineEnd = NULL;
  int nLen = read_next_line(szData, &pLineStart, &pLineEnd, &nFullLine);
  EXPECT_STREQ(pLineStart, "May");
  EXPECT_EQ(pLineEnd, pLineStart + 3);
  EXPECT_EQ(nLen, 3);
  EXPECT_EQ(strlen(pLineStart), 3);
  EXPECT_EQ(nFullLine, 1);

  nLen = read_next_line(pLineEnd + 1, &pLineStart, &pLineEnd, &nFullLine);
  EXPECT_STREQ(pLineStart, " 06 19:39:58 hitrade1");
  EXPECT_EQ(pLineEnd, pLineStart + 21);
  EXPECT_EQ(nLen, 21);
  EXPECT_EQ(strlen(pLineStart), 21);
  EXPECT_EQ(nFullLine, 1);

  nLen = read_next_line(pLineEnd + 1, &pLineStart, &pLineEnd, &nFullLine);
  EXPECT_STREQ(pLineStart, "");
  EXPECT_EQ(pLineEnd, pLineStart);
  EXPECT_EQ(nLen, 0);
  EXPECT_EQ(strlen(pLineStart), 0);
  EXPECT_EQ(nFullLine, 0);

  nLen = read_next_line(pLineEnd + 1, &pLineStart, &pLineEnd, &nFullLine);
  EXPECT_STREQ(pLineStart, "");
  EXPECT_EQ(pLineEnd, pLineStart);
  EXPECT_EQ(nLen, 0);
  EXPECT_EQ(strlen(pLineStart), 0);
  EXPECT_EQ(nFullLine, 0);
}


