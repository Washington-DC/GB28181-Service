/*****************************************************************//**
 * \file   WinApi.h
 * \brief  WINDOWS 下的一些时间转换函数的linux实现
 * 
 * \author yszs
 * \date   March 2024
 *********************************************************************/

#include <stdint.h>

#ifndef _WIN32
#include <sys/time.h>

typedef uint32_t DWORD;

typedef uint16_t WORD;

typedef uint8_t BYTE;
typedef uint8_t UCHAR;

typedef int32_t LONG;

typedef int64_t LONGLONG;

typedef uint64_t ULONGLONG;

typedef  uint64_t UINT64;

typedef union _ULARGE_INTEGER {
    struct {
        DWORD LowPart;
        DWORD HighPart;
    };
    ULONGLONG QuadPart;
} ULARGE_INTEGER;


typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;


typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;


typedef struct tagRGBQUAD {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER,  *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;


typedef struct tagBITMAPINFOHEADER{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;



void GetLocalTime(LPSYSTEMTIME lst);
FILETIME GetCurrentFileTime();
void TimevalToSystemTime(struct timeval* tv,LPSYSTEMTIME lst);
void FiletimeToTimeval(const FILETIME* ft, struct timeval *tv);
void TimevalToFiletime(const timeval* tv, LPFILETIME ft);
bool FileTimeToSystemTime(const FILETIME* ft,LPSYSTEMTIME lst);
bool SystemTimeToFileTime(const SYSTEMTIME* st,LPFILETIME lft);
bool FileTimeToLocalFileTime(const FILETIME* lpFileTime,LPFILETIME lpLocalFileTime);
bool LocalFileTimeToFileTime(const FILETIME *lpLocalFileTime, LPFILETIME lpFileTime);
void TimeSpecToFileTime(const struct timespec* ts, LPFILETIME ft);
void TimeToFileTime(const int64_t t, LPFILETIME ft);
int64_t FileTimeToTime(const FILETIME*  ft);
struct timespec FileTimeToTimeSpec(const FILETIME*  ft);


#endif