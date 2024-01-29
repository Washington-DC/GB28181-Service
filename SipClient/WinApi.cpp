#include "pch.h"
#include "WinApi.h"

#ifndef _WIN32

#define MODERNITYSECONDS 11644473600ull
#define HECTONANOSECONDS 10000000ull

void GetLocalTime(LPSYSTEMTIME lst) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    TimevalToSystemTime(&tv, lst);
}

FILETIME GetCurrentFileTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    FILETIME ft;
    TimevalToFiletime(&tv, &ft);
    return ft;
}

void TimevalToSystemTime(struct timeval *tv, LPSYSTEMTIME pst) {
    auto tm = localtime(&tv->tv_sec);
    pst->wYear = tm->tm_year + 1900;
    pst->wMonth = tm->tm_mon + 1;
    pst->wDayOfWeek = tm->tm_wday;
    pst->wDay = tm->tm_mday;
    pst->wHour = tm->tm_hour;
    pst->wMinute = tm->tm_min;
    pst->wSecond = tm->tm_sec;
    pst->wMilliseconds = tv->tv_usec / 1000;
}

void FiletimeToTimeval(const FILETIME *ft, struct timeval *tv) {
    int64_t ff = (uint64_t)((uint64_t)ft->dwHighDateTime << 32) + (uint64_t)ft->dwLowDateTime;
    tv->tv_sec = (time_t)(ff / 10000000ull - 11644473600ull);
    tv->tv_usec = (int)(ff % 10000000) / 10;
}

bool FileTimeToSystemTime(const FILETIME *ft, LPSYSTEMTIME lst) {
    struct timeval tv;
    FiletimeToTimeval(ft, &tv);
    TimevalToSystemTime(&tv, lst);
    return true;
}

bool SystemTimeToFileTime(const SYSTEMTIME *st, LPFILETIME lft) {
    struct tm t;
    t.tm_year = st->wYear - 1900;
    t.tm_mon = st->wMonth - 1;
    t.tm_mday = st->wDay;
    t.tm_hour = st->wHour;
    t.tm_min = st->wMinute;
    t.tm_sec = st->wSecond;
    t.tm_isdst = 0;

    auto ret = mktime(&t);
    if (ret != -1) {
        FILETIME ft;
        TimeToFileTime(ret, &ft);
        ft.dwLowDateTime += st->wMilliseconds * 1000 * 10;
        *lft = ft;
        // FileTimeToLocalFileTime(&ft, lft);
        return true;
    } else
        return false;
}

void TimevalToFiletime(const timeval *tv, LPFILETIME ft) {
    uint64_t t2 = tv->tv_sec * HECTONANOSECONDS + tv->tv_usec * 10 + MODERNITYSECONDS * HECTONANOSECONDS;

    ft->dwLowDateTime = (uint32_t)t2;
    ft->dwHighDateTime = (uint32_t)(t2 >> 32);
}

void TimeSpecToFileTime(const struct timespec *ts, LPFILETIME ft) {
    uint64_t x;
    x = MODERNITYSECONDS;
    x += ts->tv_sec * HECTONANOSECONDS;
    x += ts->tv_nsec / 100;
    ft->dwLowDateTime = (uint32_t)x;
    ft->dwHighDateTime = (uint32_t)(x >> 32);
}

void TimeToFileTime(const int64_t t, LPFILETIME ft) {
    uint64_t x = (t + MODERNITYSECONDS) * HECTONANOSECONDS;
    ft->dwLowDateTime = (uint32_t)x;
    ft->dwHighDateTime = (uint32_t)(x >> 32);
}

int64_t FileTimeToTime(const FILETIME *ft) {
    uint64_t t = (uint64_t)((uint64_t)ft->dwHighDateTime << 32) + (uint64_t)ft->dwLowDateTime;
    return (t - MODERNITYSECONDS * HECTONANOSECONDS) / HECTONANOSECONDS;
}

struct timespec FileTimeToTimeSpec(const FILETIME *ft) {
    uint64_t x = (uint64_t)((uint64_t)ft->dwHighDateTime << 32) + (uint64_t)ft->dwLowDateTime;
    return (struct timespec){x / HECTONANOSECONDS - MODERNITYSECONDS, x % HECTONANOSECONDS * 100};
}

bool FileTimeToLocalFileTime(const FILETIME *lpFileTime, LPFILETIME lpLocalFileTime) {
    if (NULL == lpFileTime || NULL == lpLocalFileTime)
        return false;

    time_t tt = 0;
    struct tm *ptm = NULL;
    tt = ::time(&tt);
    ptm = ::localtime(&tt);
    int nHourLocal = ptm->tm_hour;
    ptm = ::gmtime(&tt);
    int nHourUTC = ptm->tm_hour;
    int nTimeZoom = nHourLocal - nHourUTC;

    ULARGE_INTEGER uli = {lpFileTime->dwLowDateTime, lpFileTime->dwHighDateTime};
    uli.QuadPart += ((ULONGLONG)nTimeZoom * 60 * 60 * 1000 * 1000 * 10);
    lpLocalFileTime->dwLowDateTime = uli.LowPart;
    lpLocalFileTime->dwHighDateTime = uli.HighPart;

    return true;
}

bool LocalFileTimeToFileTime(const FILETIME *lpLocalFileTime, LPFILETIME lpFileTime) {
    if (NULL == lpLocalFileTime || NULL == lpFileTime)
        return false;

    time_t tt = 0;
    struct tm *ptm = NULL;
    tt = ::time(&tt);
    ptm = ::localtime(&tt);
    int nHourLocal = ptm->tm_hour;
    ptm = ::gmtime(&tt);
    int nHourUTC = ptm->tm_hour;
    int nTimeZoom = nHourLocal - nHourUTC;

    ULARGE_INTEGER uli = {lpLocalFileTime->dwLowDateTime, lpLocalFileTime->dwHighDateTime};
    uli.QuadPart -= ((ULONGLONG)nTimeZoom * 60 * 60 * 1000 * 1000 * 10);
    lpFileTime->dwLowDateTime = uli.LowPart;
    lpFileTime->dwHighDateTime = uli.HighPart;

    return true;
}

#endif