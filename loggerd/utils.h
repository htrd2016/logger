#ifndef __UTILS_H_INCLUDE__
#define __UTILS_H_INCLUDE__

// char* getNextLine(char *szInSrc, int *nInSrcLength, char *outLine, int
// *outLineLength);
#include <time.h>

typedef char BYTE;
typedef unsigned char uchar;
typedef unsigned int  uint32;

#if !defined(__cplusplus)
typedef unsigned char bool;
#define true 1
#define false 0
#endif

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

char *get_current_time(char t[]);

int get_line(uchar *buffer, int buffer_length, uchar **next_line,
                  int *remaning_length, int *is_full_line);

bool get_end_half_line(const uchar *str_src_start, const uchar *str_src_end,
                       uchar **out_last_line_start, uchar **out_last_lineEnd,
                       int *have_multi_lines);

// 2016/02/01 08:18:40
int format_time1(char *szTime, time_t *out_time);

// 20151214 20:32:08
int format_time2(char *time, time_t *out_time);

char *get_data_at(const char *szInSrc, int nInSrcLength, int nInIndex,
                  char *outData);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__UTILS_H_INCLUDE__
