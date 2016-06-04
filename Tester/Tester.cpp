#include "gtest/gtest.h"
#include <arpa/inet.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <unistd.h>

#include "memtypes.h"
#include "mylog.h"
#include "utils.h"

#define MSG_MAX_LENGTH 16
#define SIG_USER 14

ConfData configData;

void Test_Typedef() {
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
}

TEST(TestTypedefs, 1) {
  configData.block_amount = 10;
  configData.buffer_amount_in_block = 3;
  configData.size_of_buffer = 1024;
  configData.thread_amount = 3;

  Test_Typedef();
}

TEST(TestTypedefs, 2) {
  configData.block_amount = 20;
  configData.buffer_amount_in_block = 5;
  configData.size_of_buffer = 1024 * 1024;
  configData.thread_amount = 10;

  Test_Typedef();
}

TEST(TestTypedefs, 3) {
  configData.block_amount = 1;
  configData.buffer_amount_in_block = 1;
  configData.size_of_buffer = 1;
  configData.thread_amount = 1;

  Test_Typedef();
}

TEST(get_end_half_line, 0) {
  uchar data[512] = "log0";
  uchar *pLineStart = NULL;
  uchar *pLineEnd = NULL;
  int haveMultiLine = 0;
  bool bRet = get_end_half_line(data, data + strlen((char *)data), &pLineStart,
                                &pLineEnd, &haveMultiLine);
  EXPECT_EQ(bRet, true);
  EXPECT_EQ(haveMultiLine, false);
  EXPECT_EQ(pLineStart, data);
  EXPECT_EQ(strcmp((char *)pLineStart, "log0"), 0);
}

TEST(get_end_half_line, 0_1) {
  uchar data[512] = "log0";
  uchar *pLineStart = NULL;
  uchar *pLineEnd = NULL;
  int haveMultiLine = 0;
  bool bRet =
      get_end_half_line(data, data + 2, &pLineStart, &pLineEnd, &haveMultiLine);
  EXPECT_EQ(bRet, true);
  EXPECT_EQ(haveMultiLine, false);
  EXPECT_EQ(pLineStart, data);

  EXPECT_EQ(pLineEnd - pLineStart + 1, 3);
}

TEST(get_end_half_line, 0_2) {
  uchar data[512] = "\r\n";
  uchar *pLineStart = NULL;
  uchar *pLineEnd = NULL;
  int haveMultiLine = 0;
  bool bRet =
      get_end_half_line(data, data + 2, &pLineStart, &pLineEnd, &haveMultiLine);
  EXPECT_EQ(bRet, false);
  EXPECT_EQ(haveMultiLine, false);
  EXPECT_EQ(pLineStart, data + 2);
  EXPECT_EQ(strcmp((char *)pLineStart, ""), 0);
}

TEST(get_end_half_line, 0_3) {
  uchar data[512] = "\r\n\r\n";
  uchar *pLineStart = NULL;
  uchar *pLineEnd = NULL;
  int haveMultiLine = 0;
  bool bRet =
      get_end_half_line(data, data + 4, &pLineStart, &pLineEnd, &haveMultiLine);
  EXPECT_EQ(bRet, false);
  EXPECT_EQ(haveMultiLine, true);
  EXPECT_EQ(pLineStart, data + 4);
  EXPECT_EQ(strcmp((char *)pLineStart, ""), 0);
}

TEST(get_end_half_line, 0_4) {
  uchar data[512] = "\r\n\r";
  uchar *pLineStart = NULL;
  uchar *pLineEnd = NULL;
  int haveMultiLine = 0;
  bool bRet =
      get_end_half_line(data, data + 3, &pLineStart, &pLineEnd, &haveMultiLine);
  EXPECT_EQ(bRet, false);
  EXPECT_EQ(haveMultiLine, true);
  EXPECT_EQ(pLineStart, data + 3);
  EXPECT_EQ(strcmp((char *)pLineStart, ""), 0);
}

TEST(get_end_half_line, 0_5) {
  uchar data[512] = "\r\r";
  uchar *pLineStart = NULL;
  uchar *pLineEnd = NULL;
  int haveMultiLine = 0;
  bool bRet =
      get_end_half_line(data, data + 2, &pLineStart, &pLineEnd, &haveMultiLine);
  EXPECT_EQ(bRet, false);
  EXPECT_EQ(haveMultiLine, true);
  EXPECT_EQ(pLineStart, data + 2);
  EXPECT_EQ(strcmp((char *)pLineStart, ""), 0);
}

TEST(get_end_half_line, 1) {
  int haveMultiLine = 0;
  uchar data[512] = "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\n"
                    "log0\r\nlog0\r\nlog1";
  uchar *pLineStart = NULL;
  uchar *pLineEnd = NULL;
  bool bRet = get_end_half_line(data, data + strlen((char *)data), &pLineStart,
                                &pLineEnd, &haveMultiLine);
  EXPECT_EQ(bRet, true);
  EXPECT_EQ(haveMultiLine, true);
  EXPECT_EQ(pLineStart, data + strlen((char *)data) - strlen("log1"));
  EXPECT_EQ(strcmp((char *)pLineStart, "log1"), 0);
}

TEST(get_end_half_line, 2) {
  uchar data[512] = "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\n"
                    "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\n";
  uchar *pLineStart = NULL;
  uchar *pLineEnd = NULL;
  int haveMultiLine = 0;
  bool bRet = get_end_half_line(data, data + strlen((char *)data), &pLineStart,
                                &pLineEnd, &haveMultiLine);
  EXPECT_EQ(bRet, false);
  EXPECT_EQ(haveMultiLine, true);
  EXPECT_EQ(pLineStart, data + strlen((char *)data));
  EXPECT_EQ(strcmp((char *)pLineStart, ""), 0);
}

TEST(get_end_half_line, 2_0) {
  uchar data[512] = "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\n"
                    "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r";
  uchar *pLineStart = NULL;
  uchar *pLineEnd = NULL;
  int haveMultiLine = 0;
  bool bRet = get_end_half_line(data, data + strlen((char *)data), &pLineStart,
                                &pLineEnd, &haveMultiLine);
  EXPECT_EQ(bRet, false);
  EXPECT_EQ(haveMultiLine, true);
  EXPECT_EQ(pLineStart, data + strlen((char *)data));
  EXPECT_EQ(strcmp((char *)pLineStart, ""), 0);
}

TEST(get_end_half_line, 2_1) {
  uchar data[512] = "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\n"
                    "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r";
  uchar *pLineStart = NULL;
  uchar *pLineEnd = NULL;
  int haveMultiLine = 0;
  bool bRet = get_end_half_line(data, data + 26 - 1, &pLineStart, &pLineEnd,
                                &haveMultiLine);
  EXPECT_EQ(bRet, true);
  EXPECT_EQ(haveMultiLine, true);
  EXPECT_EQ(pLineStart, data + 25 - 1);

  EXPECT_EQ(pLineEnd - pLineStart + 1, 2);
}

TEST(get_end_half_line, 2_2) {
  uchar data[512] = "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\n"
                    "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\n\n\n";
  uchar *pLineStart = NULL;
  uchar *pLineEnd = NULL;
  int haveMultiLine = 0;
  bool bRet = get_end_half_line(data, data + strlen((char *)data), &pLineStart,
                                &pLineEnd, &haveMultiLine);
  EXPECT_EQ(bRet, false);
  EXPECT_EQ(haveMultiLine, true);
  EXPECT_EQ(pLineStart, data + strlen((char *)data));
  EXPECT_EQ(strcmp((char *)pLineStart, ""), 0);
  EXPECT_EQ(pLineStart, pLineEnd);
}

TEST(get_end_half_line, 2_3) {
  uchar data[512] = "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\n"
                    "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\n\r\n\n";
  uchar *pLineStart = NULL;
  uchar *pLineEnd = NULL;
  int haveMultiLine = 0;
  bool bRet = get_end_half_line(data, data + strlen((char *)data), &pLineStart,
                                &pLineEnd, &haveMultiLine);
  EXPECT_EQ(bRet, false);
  EXPECT_EQ(haveMultiLine, true);
  EXPECT_EQ(pLineStart, data + strlen((char *)data));
  EXPECT_EQ(strcmp((char *)pLineStart, ""), 0);
  EXPECT_EQ(pLineStart, pLineEnd);
}

TEST(get_end_half_line, 2_4) {
  uchar data[512] = "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\n"
                    "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\n\r\n\r\n";
  uchar *pLineStart = NULL;
  uchar *pLineEnd = NULL;
  int haveMultiLine = 0;
  bool bRet = get_end_half_line(data, data + strlen((char *)data), &pLineStart,
                                &pLineEnd, &haveMultiLine);
  EXPECT_EQ(bRet, false);
  EXPECT_EQ(haveMultiLine, true);
  EXPECT_EQ(pLineStart, data + strlen((char *)data));
  EXPECT_EQ(strcmp((char *)pLineStart, ""), 0);
  EXPECT_EQ(pLineStart, pLineEnd);
}

TEST(get_end_half_line, 2_5) {
  uchar data[512] = "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\n"
                    "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\n\r\n\r\n\r";
  uchar *pLineStart = NULL;
  uchar *pLineEnd = NULL;
  int haveMultiLine = 0;
  bool bRet = get_end_half_line(data, data + strlen((char *)data), &pLineStart,
                                &pLineEnd, &haveMultiLine);
  EXPECT_EQ(bRet, false);
  EXPECT_EQ(haveMultiLine, true);
  EXPECT_EQ(pLineStart, data + strlen((char *)data));
  EXPECT_EQ(strcmp((char *)pLineStart, ""), 0);
  EXPECT_EQ(pLineStart, pLineEnd);
}

TEST(get_end_half_line, 2_6) {
  uchar data[512] = "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\n"
                    "log0\r\nlog0\r\nlog0\r\nlog0\r\nlog0\r\n\r\n\r\n\r\r";
  uchar *pLineStart = NULL;
  uchar *pLineEnd = NULL;
  int haveMultiLine = 0;
  bool bRet = get_end_half_line(data, data + strlen((char *)data), &pLineStart,
                                &pLineEnd, &haveMultiLine);
  EXPECT_EQ(bRet, false);
  EXPECT_EQ(haveMultiLine, true);
  EXPECT_EQ(pLineStart, data + strlen((char *)data));
  EXPECT_EQ(strcmp((char *)pLineStart, ""), 0);
  EXPECT_EQ(pLineStart, pLineEnd);
}

TEST(get_line, 0) {
  int isFullLine = 0;
  uchar data[1024] = "May\n 06 19:39:58 hitrade1\r\n Receive Nak";
  int remaning_length = strlen((char *)data);

  uchar *pLineStart = NULL;
  uchar *ptr = data;
  int nLen = get_line(ptr, remaning_length, &pLineStart, &remaning_length,
                      &isFullLine);
  EXPECT_STREQ((char *)pLineStart, "May");
  EXPECT_EQ(pLineStart, ptr);
  EXPECT_STREQ((char *)pLineStart + nLen,
               " 06 19:39:58 hitrade1\r\n Receive Nak");
  EXPECT_EQ(nLen, 4);
  EXPECT_EQ(remaning_length, 35);
  EXPECT_EQ(isFullLine, 1);

  ptr += nLen;
  nLen = get_line(ptr, remaning_length, &pLineStart, &remaning_length,
                  &isFullLine);
  EXPECT_STREQ((char *)pLineStart, " 06 19:39:58 hitrade1");
  EXPECT_EQ(pLineStart, ptr);
  EXPECT_EQ(nLen, 23);
  EXPECT_EQ(remaning_length, 12);
  EXPECT_EQ(isFullLine, 1);

  ptr += nLen;
  nLen = get_line(ptr, remaning_length, &pLineStart, &remaning_length,
                  &isFullLine);
  EXPECT_STREQ((char *)pLineStart, " Receive Nak");
  EXPECT_EQ(pLineStart, ptr);
  EXPECT_EQ(nLen, 12);
  EXPECT_EQ(remaning_length, 0);
  EXPECT_EQ(strlen((char *)pLineStart), 12);
  EXPECT_EQ(isFullLine, 0);

  ptr += nLen;
  nLen = get_line(ptr, remaning_length, &pLineStart, &remaning_length,
                  &isFullLine);
  EXPECT_STREQ((char *)pLineStart, "");
  EXPECT_EQ(nLen, 0);
  EXPECT_EQ(remaning_length, 0);
  EXPECT_EQ(strlen((char *)pLineStart), 0);
  EXPECT_EQ(isFullLine, 0);
}

TEST(get_line, 0_1) {
  int isFullLine = 0;
  uchar data[1024] = "May\r\n 06 19:39:58 hitrade1\r\n\r Receive Nak";
  int remaning_length = strlen((char *)data);

  uchar *pLineStart = NULL;
  uchar *ptr = data;
  int nLen = get_line(ptr, remaning_length, &pLineStart, &remaning_length,
                      &isFullLine);
  EXPECT_STREQ((char *)pLineStart, "May");
  EXPECT_EQ(pLineStart, ptr);
  EXPECT_STREQ((char *)pLineStart + nLen,
               " 06 19:39:58 hitrade1\r\n\r Receive Nak");
  EXPECT_EQ(nLen, 5);
  EXPECT_EQ(remaning_length, 36);
  EXPECT_EQ(isFullLine, 1);

  ptr += nLen;
  nLen = get_line(ptr, remaning_length, &pLineStart, &remaning_length,
                  &isFullLine);
  EXPECT_STREQ((char *)pLineStart, " 06 19:39:58 hitrade1");
  EXPECT_EQ(pLineStart, ptr);
  EXPECT_EQ(nLen, 24);
  EXPECT_EQ(remaning_length, 12);
  EXPECT_EQ(isFullLine, 1);

  ptr += nLen;
  nLen = get_line(ptr, remaning_length, &pLineStart, &remaning_length,
                  &isFullLine);
  EXPECT_STREQ((char *)pLineStart, " Receive Nak");
  EXPECT_EQ(pLineStart, ptr);
  EXPECT_EQ(nLen, 12);
  EXPECT_EQ(remaning_length, 0);
  EXPECT_EQ(strlen((char *)pLineStart), 12);
  EXPECT_EQ(isFullLine, 0);

  ptr += nLen;
  nLen = get_line(ptr, remaning_length, &pLineStart, &remaning_length,
                  &isFullLine);
  EXPECT_STREQ((char *)pLineStart, "");
  EXPECT_EQ(nLen, 0);
  EXPECT_EQ(remaning_length, 0);
  EXPECT_EQ(strlen((char *)pLineStart), 0);
  EXPECT_EQ(isFullLine, 0);
}

TEST(get_line, 1) {
  int nFullLine = 0;
  uchar data[1024 * 2] =
      "Copyright (c) 2015, Brett824 and Hyphen-ated\n          All rights "
      "reserved.\r\n          Redistribution and use in source and binary "
      "forms, with or without modification, are permitted provided that the "
      "following conditions are met:\n          1. Redistributions of source "
      "code must retain the above copyright notice, this list of conditions "
      "and the following disclaimer.\r\n          2. Redistributions in binary "
      "form must reproduce the above copyright notice, this list of conditions "
      "and the following disclaimer in the documentation and/or other "
      "materials provided with the distribution.\n          THIS SOFTWARE IS "
      "PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS AS IS AND ANY "
      "EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE "
      "IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR "
      "PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR "
      "CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, "
      "EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, "
      "PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR "
      "PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF "
      "LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING "
      "NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS "
      "SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";
  int remaning_length = strlen((char *)data);

  uchar *pLineStart = NULL;
  int nLen = get_line(data, remaning_length, &pLineStart, &remaning_length,
                      &nFullLine);
  EXPECT_STREQ((char *)pLineStart, (char *)data);
  EXPECT_STREQ((char *)pLineStart,
               "Copyright (c) 2015, Brett824 and Hyphen-ated");
  EXPECT_EQ(nLen, 45);
  EXPECT_EQ(strlen((char *)pLineStart) + 1, nLen);
  EXPECT_EQ(nFullLine, 1);

  nLen = get_line(pLineStart + nLen, remaning_length, &pLineStart,
                  &remaning_length, &nFullLine);
  EXPECT_STREQ((char *)pLineStart, "          All rights reserved.");
  EXPECT_EQ(nLen, 32);
  EXPECT_EQ(strlen((char *)pLineStart) + 2, nLen);
  EXPECT_EQ(nFullLine, 1);

  nLen = get_line(pLineStart + nLen, remaning_length, &pLineStart,
                  &remaning_length, &nFullLine);
  EXPECT_STREQ((char *)pLineStart, "          Redistribution and use in source "
                                   "and binary forms, with or without "
                                   "modification, are permitted provided that "
                                   "the following conditions are met:");
  EXPECT_EQ(nLen, 153);
  EXPECT_EQ(strlen((char *)pLineStart) + 1, nLen);
  EXPECT_EQ(nFullLine, 1);

  nLen = get_line(pLineStart + nLen, remaning_length, &pLineStart,
                  &remaning_length, &nFullLine);
  EXPECT_STREQ((char *)pLineStart, "          1. Redistributions of source "
                                   "code must retain the above copyright "
                                   "notice, this list of conditions and the "
                                   "following disclaimer.");
  EXPECT_EQ(nLen, 139);
  EXPECT_EQ(strlen((char *)pLineStart) + 2, nLen);
  EXPECT_EQ(nFullLine, 1);

  nLen = get_line(pLineStart + nLen, remaning_length, &pLineStart,
                  &remaning_length, &nFullLine);
  EXPECT_STREQ((char *)pLineStart,
               "          2. Redistributions in binary form must reproduce the "
               "above copyright notice, this list of conditions and the "
               "following disclaimer in the documentation and/or other "
               "materials provided with the distribution.");
  EXPECT_EQ(nLen, 216);
  EXPECT_EQ(strlen((char *)pLineStart) + 1, nLen);
  EXPECT_EQ(nFullLine, 1);

  nLen = get_line(pLineStart + nLen, remaning_length, &pLineStart,
                  &remaning_length, &nFullLine);
  EXPECT_STREQ((char *)pLineStart,
               "          THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "
               "AND CONTRIBUTORS AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES, "
               "INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF "
               "MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE "
               "DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR "
               "CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, "
               "SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT "
               "NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; "
               "LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) "
               "HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN "
               "CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR "
               "OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, "
               "EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.");
  EXPECT_EQ(nLen, 763);
  EXPECT_EQ(strlen((char *)pLineStart), nLen);
  EXPECT_EQ(nFullLine, 0);

  uchar buf[1024] = "wet\n123456";
  remaning_length = 2;
  nLen =
      get_line(buf, remaning_length, &pLineStart, &remaning_length, &nFullLine);
  EXPECT_STREQ((char *)pLineStart, "wet\n123456");
  EXPECT_EQ(nLen, 2);
  EXPECT_EQ(strlen((char *)pLineStart), 10);
  EXPECT_EQ(nFullLine, 0);
}

// void sigUserProc(int) {
//  int status;
//  wait(&status); //接收子进程退出
//}

#ifdef __CLANG__
namespace testing {
std::string Message::GetString() const { return ""; }
}
#endif //__CLANG__

void Test_Connection(int amount) {
  int i = 0;
  short port = 12345;

  char server_ip[] = "127.0.0.1";
  char buf[MSG_MAX_LENGTH];

  ssize_t sended_out_length = 0;
  int *socks;

  struct sockaddr_in remote_addr;
  memset(&remote_addr, 0, sizeof(remote_addr));
  remote_addr.sin_family = AF_INET;
  remote_addr.sin_addr.s_addr = inet_addr(server_ip);
  remote_addr.sin_port = htons(port);
  //sleep(2);
  socks = new int[amount];
  for (; i < amount; i++) {
    socks[i] = socket(AF_INET, SOCK_STREAM, 0);
    EXPECT_GT(socks[i], 0);

    if (socks[i] > 0) {
      int ret = connect(socks[i], (struct sockaddr *)&remote_addr,
                        sizeof(struct sockaddr));
      EXPECT_EQ(ret, 0);
      //printf("%d connect result %s\n", i, ((ret == 0) ? "Good" : "Bad"));
    }
  }
  //sleep(2);

  for (i = 0; i < amount; ++i) {
        //printf("%d send ... ", i);
        sended_out_length = send(socks[i], buf, MSG_MAX_LENGTH, 0);
        //int recv_length = recv(socks[i], buf, MSG_MAX_LENGTH, 0);
        //printf(" recv ...");
        EXPECT_EQ(sended_out_length, MSG_MAX_LENGTH);
        //EXPECT_EQ(recv_length, MSG_MAX_LENGTH);
        //printf(" done\n");
  }
  //sleep(2);

  for (i = 0; i < amount; i++) {
      shutdown(socks[i], SHUT_RDWR);
      close(socks[i]);
      //printf("close %d\n", i);
  }

  delete[] socks;
}

TEST(server_Connection, 1) {
  for (int i = 0; i < 20; ++i)
    Test_Connection(10);
}

TEST(server_Connection, 2) {
  for (int i = 0; i < 20; ++i)
    Test_Connection(1);
}

TEST(server_Connection, 3) {
  for (int i = 0; i < 20; ++i)
    Test_Connection(8);
}

TEST(server_Connection, 4) {
  for (int i = 0; i < 20; ++i)
    Test_Connection(4);
}

TEST(server_Connection, 5) {
  for (int i = 0; i < 20; ++i)
    Test_Connection(10);
}

#define thread_amount 10
typedef struct thread_param { uint8_t id; } thread_param;

static uint8_t bad[thread_amount];

void *TcpClient(void *param) {
  //    char servpath[] = "/home/yang/workspace/logger/tcp_server/tcp_server";
  short port = 12345;

  char server_ip[] = "127.0.0.1";
  char send_buf[MSG_MAX_LENGTH] = "msg";
  char recv_buf[MSG_MAX_LENGTH];

  ssize_t sended_out_length = 0;
  int sockfd = 0;

  struct sockaddr_in remote_addr;
  memset(&remote_addr, 0, sizeof(remote_addr));
  remote_addr.sin_family = AF_INET;
  remote_addr.sin_addr.s_addr = inet_addr(server_ip);
  remote_addr.sin_port = htons(port);

  bad[((thread_param *)param)->id] = 0;
  memset(recv_buf, ((thread_param *)param)->id, MSG_MAX_LENGTH);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  EXPECT_GT(sockfd, 0);
  if (sockfd <= 0)
    goto bad_return;

  if (sockfd > 0) {
    int ret = connect(sockfd, (struct sockaddr *)&remote_addr,
                      sizeof(struct sockaddr));
    EXPECT_EQ(ret, 0);
    if (ret == 0) {
      struct timeval timeout;
      timeout.tv_sec = 5;
      timeout.tv_usec = 0;

      if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                     sizeof(timeout)) < 0)
        perror("setsockopt failed\n");

      if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                     sizeof(timeout)) < 0)
        perror("setsockopt failed\n");

      sended_out_length = send(sockfd, send_buf, MSG_MAX_LENGTH, 0);
      //recv_length = recv(sockfd, recv_buf, MSG_MAX_LENGTH, 0);

      ret = (sended_out_length == MSG_MAX_LENGTH);
      EXPECT_NE(ret, 0);
      if (!ret) {
        printf("thread id %d , test sended_out_length == MSG_MAX_LENGTH\n", ((thread_param *)param)->id);
        goto bad_return;
      }
      //ret = (recv_length == MSG_MAX_LENGTH);
      //EXPECT_NE(ret, 0);
      //if (!ret) {
      //  printf("thread id %d , recv_length == MSG_MAX_LENGTH\n", ((thread_param *)param)->id);
      //  goto bad_return;
      //}

      //ret = memcmp(recv_buf, send_buf, MSG_MAX_LENGTH);
      //EXPECT_EQ(ret, 0);
      //if (ret) {
      //  printf("thread id %d , memcmp\n", ((thread_param *)param)->id);
      //  goto bad_return;
      //}

      //sended_out_length = send(sockfd, send_buf, MSG_MAX_LENGTH, 0);
      //recv_length = recv(sockfd, recv_buf, MSG_MAX_LENGTH, 0);
      //ret = (sended_out_length == MSG_MAX_LENGTH);
      //EXPECT_NE(ret, 0);
      //if (!ret)
      //  goto bad_return;
      //
      //ret = (recv_length == MSG_MAX_LENGTH);
      //EXPECT_NE(ret, 0);
      //if (!ret)
      //  goto bad_return;
      //
      //ret = memcmp(recv_buf, send_buf, MSG_MAX_LENGTH);
      //EXPECT_EQ(ret, 0);
      //if (ret)
      //  goto bad_return;
    } else
      goto bad_return;
  } else
    goto bad_return;
  shutdown(sockfd, SHUT_RDWR);
  close(sockfd);
  //pthread_detach(pthread_self());
  return (0);
bad_return:
  bad[((thread_param *)param)->id] = 1;
  shutdown(sockfd, SHUT_RDWR);
  close(sockfd);
  //pthread_detach(pthread_self());
  return (0);
}

void TestTcpClient() {
    thread_param param[thread_amount];
    pthread_t t[thread_amount];
    memset(bad, 0, thread_amount);
    for (int i = 0; i < thread_amount; i++) {
      param[i].id = (uint8_t)i;
      int ret = pthread_create(t + i, 0, TcpClient, param + i);
      EXPECT_EQ(ret, 0);
    }

    for (int i = 0; i < thread_amount; i++)
      pthread_join(t[i], NULL);

    for (int i = 0; i < thread_amount; i++)
      EXPECT_EQ(bad[i], 0);
}

TEST(TcpClient, 1_0) {
    TestTcpClient();
}

TEST(TcpClient, 1_1) {
    TestTcpClient();
}

TEST(TcpClient, 1_2) {
    TestTcpClient();
}

TEST(TcpClient, 1_3) {
    TestTcpClient();
}

TEST(TcpClient, 1_4) {
    TestTcpClient();
}

TEST(TcpClient, 1_5) {
    TestTcpClient();
}

TEST(TcpClient, 1_6) {
    TestTcpClient();
}

TEST(TcpClient, 1_7) {
    TestTcpClient();
}


TEST(TcpClient, 1_8) {
    TestTcpClient();
}


TEST(TcpClient, 1_9) {
    TestTcpClient();
}


TEST(TcpClient, 1_10) {
    TestTcpClient();
}

