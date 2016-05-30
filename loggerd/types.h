#ifndef __TYPES_H_INCLUDE__
#define __TYPES_H_INCLUDE__

#include "utils.h"

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

//接收数据块Block:
typedef struct RecvBuffer {
  uchar *buf_start;
  uchar *buf_end;
  uchar *data_start_ptr; //数据开始指针
  uchar *data_end_ptr;   //数据结束指针
  //uchar *to_write_ptr;//要写数据位置
  bool free;
} RecvBuffer;

typedef struct Block {
  uchar *pHeadPtr;
  uchar *pTailPtr;
  RecvBuffer *recvBufs; //数组指针 buffer_amount个
  uint32 bufIndexToWrite;
  uint32 buffer_amount;
  bool   free;            // block status
} Block;

typedef struct ThreadData {
  bool free;
  bool have_data;
  //    uchar *start_data_pt;
  //    uchar *end_data_pt;
  RecvBuffer *recv_buffer;
  Block *block;
} ThreadData;

typedef struct ConfData {
  uint32      local_port;                ///listen port
  uint32      thread_amount;             ///amount of all thread
  size_t      size_of_buffer;            ///size of each buffer
  uint32      buffer_amount_in_block;    ///amount of buffer in a block
  uint32      block_amount;              ///amount of block
  ThreadData* pThreadData;               ///The array of thread data
  Block*      pBlock;                    ///The array of Block
  char        logfile[MAX_PATH];         ///the path of logfile
  bool        stop;                      ///the process should stop (1) or not (0)
  uchar *     recv_data_memory;
  uchar *     indicator_memory;
} ConfData;


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

extern ConfData configData;

/**
 * @brief init_mem, alloc and initialize memory.
 * -- Configuation Data --
 * +----------+
 * | ConfData |
 * +----------+
 * --- indicator memory ---
 * +--------------------------------------------------------+
 * | Block[0] | Block[1] |         ...           | Block[m] |
 * +--------------------------------------------------------+
 * | RecvBuff[0, 0] | RecvBuf[0, 1] |    ... | RecvBuf[0,n] |
 * +--------------------------------------------------------+
 * +--------------------------------------------------------+
 * | RecvBuff[1, 0] | RecvBuf[1, 1] |    ... | RecvBuf[1,n] |
 * +--------------------------------------------------------+
 *
 * +--------------------------------------------------------+
 * | RecvBuff[m, 0] | RecvBuf[m, 1] |    ... | RecvBuf[m,n] |
 * +--------------------------------------------------------+
 * | ThreadData[0] | ThreadData[1] |   .... | ThreadData[k] |
 * +--------------------------------------------------------+
 *
 * @return zeor of good, otherwise for bad
 */
extern int init_mem();
/**
 * @brief release_mem release all memory and clean the configData
 */
extern void release_mem();
extern ThreadData *get_thread_data(int index);
extern ThreadData *get_one_free_thread_data();
extern Block *get_free_block();
extern Block *get_block(int index);

extern void reset_block(Block *pBlock);

extern RecvBuffer *get_next_free_recv_buffer(Block *pBlock);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__TYPES_H_INCLUDE__
