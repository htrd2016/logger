#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
/**
 * @get the first line in the buffer to read,return the length of first line
 */
int get_line(uchar *buffer,
             int buffer_length,
             uchar **next_line,
             int *remaning_length,
             int *is_full_line)
{
    uchar *buf = buffer;
    uchar *pLineEndPos = buf;
    *remaning_length = buffer_length;
    *is_full_line = 0;

    //获取起始位置
    while (*buf == '\n' || *buf == '\r')
    {
        if (buf-buffer>=buffer_length)
        {
            *next_line = buf;//获取起始不为\n或\r的值
            pLineEndPos = buf;
            *is_full_line = 0;
            return 0;
        }
        buf++;
        (*remaning_length)--;
    }

    if(*buf == '\0')
    {
        *next_line = buf;//获取起始不为\n或\r的值
        pLineEndPos = buf;
        *remaning_length = 0;
        *is_full_line = 0;
        return 0;
    }

    *next_line = buf;//获取起始不为\n或\r的值

    //获取结束位置
    while (*buf != '\n' && *buf != '\0')
    {
        if (buf-buffer+1>=buffer_length)
        {
            break;
        }

        if(*buf == '\r')
        {
            *buf = '\0';
        }
        buf++;
        (*remaning_length)--;
    }

    if(*buf == '\n')
    {
        *buf = '\0';
        *is_full_line = 1;
    }
    //*src = '\0';
    (*remaning_length)--;
    pLineEndPos = buf;
    return pLineEndPos - (*next_line)+1;
}

/**
  return:true-half line,false-full line
*/
bool get_end_half_line(const uchar *buf_start, const uchar *buf_end,
                       uchar ** out_last_line_start, uchar **out_last_line_end,
                       int *have_multi_mines)
{
    uchar *p = (uchar*)buf_end;
    *out_last_line_end = (uchar*)buf_end;
    *have_multi_mines = false;

    while(*p != '\n' && p>buf_start)
    {
        p--;
    }

    if(*p == '\n')
    {
        p++;
        *have_multi_mines = true;
    }

    *out_last_line_start = p;
    if(*out_last_line_end==*out_last_line_start
            &&**out_last_line_end!='\0'
            &&**out_last_line_end!='\n')//just one char
    {
        return true;
    }
    return (*out_last_line_end-*out_last_line_start)>0?true:false;
}

//2016/02/01 08:18:40
int format_time1(char * szTime, time_t *out_time)
{
    int nRet = -1;
    struct tm tm1;
    if(sscanf(szTime, "%4d/%2d/%2d %2d:%2d:%2d",
              &tm1.tm_year,
              &tm1.tm_mon,
              &tm1.tm_mday,
              &tm1.tm_hour,
              &tm1.tm_min,
              &tm1.tm_sec)==6)
    {
        nRet = 0;
    }

    tm1.tm_year -= 1900;
    tm1.tm_mon --;
    tm1.tm_isdst=-1;

    *out_time = mktime(&tm1);
    return nRet;
}

//20151214 20:32:08
int format_time2(char * time, time_t *out_time)
{
    int bRet = -1;

    struct tm tm1;
    if (sscanf(time, "%4d%2d%2d %2d:%2d:%2d",
               &tm1.tm_year,
               &tm1.tm_mon,
               &tm1.tm_mday,
               &tm1.tm_hour,
               &tm1.tm_min,
               &tm1.tm_sec)==6)
    {
        bRet = 0;
    }

    tm1.tm_year -= 1900;
    tm1.tm_mon --;
    tm1.tm_isdst=-1;

    *out_time = mktime(&tm1);
    return bRet;
}

char *get_data_at(const char* szInSrc, int nInSrcLength, int nInIndex, char *outData)
{
    char *pDes = outData;
    char *p = (char*)szInSrc;
    int nCount = 0;
    int i = 0;
    for (; i<nInSrcLength&&nCount<nInIndex; i++)//查找第nInIndex-1次出现的位置
    {
        while (*p != '|')
        {
            p++;
            if ((p-szInSrc)>=nInSrcLength)
            {
                outData[0] = '\0';
                return outData;
            }
        }
        p++;
        nCount++;
    }

    while(p-szInSrc<nInSrcLength && *p!= '|')
    {
        *pDes++ = *p++;
    }
    *pDes = '\0';

    return outData;
}

char* get_current_time(char t[])
{
    time_t nowtime = time(NULL);
    struct tm *now = localtime(&nowtime);
    sprintf(t, "%04d-%02d-%02d %02d:%02d:%02d\nunix time: %ld\n",
            now->tm_year+1900, now->tm_mon+1, now->tm_mday,
            now->tm_hour, now->tm_min, now->tm_sec, (long)nowtime);
    return t;
}
