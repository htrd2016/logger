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

TEST(get_line, get_next_line0) {
  int isFullLine = 0;
  uchar szData[1024] = "May\n 06 19:39:58 hitrade1\r\n Receive Nak";
  int remaning_length = strlen((char*)szData);

  uchar *pLineStart = NULL;
  int nLen = get_line(szData, remaning_length, &pLineStart,
                           &remaning_length, &isFullLine);
  EXPECT_STREQ((char*)pLineStart, "May");
  EXPECT_EQ(pLineStart, szData);
  EXPECT_STREQ((char*)pLineStart+nLen, " 06 19:39:58 hitrade1\r\n Receive Nak");
  EXPECT_EQ(nLen, 4);
  EXPECT_EQ(remaning_length, 35);
  EXPECT_EQ(isFullLine, 1);

  nLen = get_line(pLineStart+nLen, remaning_length,
                       &pLineStart, &remaning_length, &isFullLine);
  EXPECT_STREQ((char*)pLineStart, " 06 19:39:58 hitrade1");
  EXPECT_EQ(pLineStart, szData + 3 + 1);
  EXPECT_EQ(nLen, 23);
  EXPECT_EQ(remaning_length, 12);
  EXPECT_EQ(isFullLine, 1);

  nLen = get_line(pLineStart+nLen,remaning_length,
                       &pLineStart, &remaning_length, &isFullLine);
  EXPECT_STREQ((char*)pLineStart, " Receive Nak");
  EXPECT_EQ(pLineStart, szData + 3 + 1 + 21 + 2);
  EXPECT_EQ(nLen, 12);
  EXPECT_EQ(remaning_length, 0);
  EXPECT_EQ(strlen((char*)pLineStart), 12);
  EXPECT_EQ(isFullLine, 0);

  nLen = get_line(pLineStart+nLen,remaning_length, &pLineStart, &remaning_length, &isFullLine);
  EXPECT_STREQ((char*)pLineStart, "");
  EXPECT_EQ(nLen, 0);
  EXPECT_EQ(remaning_length, 0);
  EXPECT_EQ(strlen((char*)pLineStart), 0);
  EXPECT_EQ(isFullLine, 0);
}

TEST(get_line, get_next_line1) {
  int nFullLine = 0;
  uchar szData[1024] = "May\n 06 19:39:58 hitrade1\r\n Receive Nak";

  uchar *pLineStart = NULL;
  int remaning_length = strlen((char *)szData);
  int nLen =
      get_line(szData, remaning_length, &pLineStart, &remaning_length, &nFullLine);
  EXPECT_EQ(pLineStart, szData);
  EXPECT_EQ(nLen, 4);
  EXPECT_EQ(remaning_length, 35);
  EXPECT_EQ(strlen((char*)pLineStart)+1, nLen);
  EXPECT_EQ(nFullLine, 1);

  pLineStart = pLineStart + nLen;
  nLen = get_line(pLineStart, remaning_length, &pLineStart, &remaning_length, &nFullLine);
  EXPECT_STREQ((char *)pLineStart, (char *)szData + 4);
  EXPECT_EQ(nLen, 23);
  EXPECT_EQ(remaning_length, 12);
  EXPECT_EQ(nFullLine, 1);

  pLineStart = pLineStart + nLen;
  nLen = get_line(pLineStart, remaning_length, &pLineStart, &remaning_length, &nFullLine);
  EXPECT_STREQ((char *)pLineStart, " Receive Nak");
  EXPECT_EQ(nLen, 12);
  EXPECT_EQ(remaning_length, 0);
  EXPECT_EQ(nFullLine, 0);
}

TEST(get_line, get_next_line2) {
  int nFullLine = 0;
  uchar szData[1024*2] = "Copyright (c) 2015, Brett824 and Hyphen-ated\n          All rights reserved.\r\n          Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:\n          1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.\r\n          2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.\n          THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";
  int remaning_length = strlen((char *)szData);

  uchar *pLineStart = NULL;
  int nLen = get_line(szData, remaning_length, &pLineStart, &remaning_length, &nFullLine);
  EXPECT_STREQ((char*)pLineStart, (char*)szData);
  EXPECT_STREQ((char*)pLineStart, "Copyright (c) 2015, Brett824 and Hyphen-ated");
  EXPECT_EQ(nLen, 45);
  EXPECT_EQ(strlen((char*)pLineStart)+1, nLen);
  EXPECT_EQ(nFullLine, 1);

  nLen = get_line(pLineStart+nLen, remaning_length, &pLineStart, &remaning_length, &nFullLine);
  EXPECT_STREQ((char*)pLineStart, "          All rights reserved.");
  EXPECT_EQ(nLen, 32);
  EXPECT_EQ(strlen((char*)pLineStart)+2, nLen);
  EXPECT_EQ(nFullLine, 1);

  nLen = get_line(pLineStart+nLen, remaning_length, &pLineStart, &remaning_length, &nFullLine);
  EXPECT_STREQ((char*)pLineStart, "          Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:");
  EXPECT_EQ(nLen, 153);
  EXPECT_EQ(strlen((char*)pLineStart)+1, nLen);
  EXPECT_EQ(nFullLine, 1);

  nLen = get_line(pLineStart+nLen, remaning_length, &pLineStart, &remaning_length, &nFullLine);
  EXPECT_STREQ((char*)pLineStart, "          1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.");
  EXPECT_EQ(nLen, 139);
  EXPECT_EQ(strlen((char*)pLineStart)+2, nLen);
  EXPECT_EQ(nFullLine, 1);

  nLen = get_line(pLineStart+nLen, remaning_length, &pLineStart, &remaning_length, &nFullLine);
  EXPECT_STREQ((char*)pLineStart, "          2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.");
  EXPECT_EQ(nLen, 216);
  EXPECT_EQ(strlen((char*)pLineStart)+1, nLen);
  EXPECT_EQ(nFullLine, 1);

  nLen = get_line(pLineStart+nLen, remaning_length, &pLineStart,
                       &remaning_length, &nFullLine);
  EXPECT_STREQ((char*)pLineStart, "          THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.");
  EXPECT_EQ(nLen, 763);
  EXPECT_EQ(strlen((char*)pLineStart), nLen);
  EXPECT_EQ(nFullLine, 0);

  uchar buf[1024] = "wet\n123456";
  remaning_length = 2;
  nLen = get_line(buf, remaning_length, &pLineStart,
                       &remaning_length, &nFullLine);
  EXPECT_STREQ((char*)pLineStart, "wet\n123456");
  EXPECT_EQ(nLen, 2);
  EXPECT_EQ(strlen((char*)pLineStart), 10);
  EXPECT_EQ(nFullLine, 0);
}
