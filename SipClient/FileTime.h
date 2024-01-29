#ifndef SRC_UTIL_FILETIME_H_
#define SRC_UTIL_FILETIME_H_
#include <string>

#ifndef _WIN32
#include "WinApi.h"
#endif

namespace toolkit {
class CFileTimeSpan {
public:
    CFileTimeSpan();
    CFileTimeSpan(const CFileTimeSpan &span);
    CFileTimeSpan(LONGLONG nSpan);

    CFileTimeSpan &operator=(const CFileTimeSpan &span);

    CFileTimeSpan &operator+=(CFileTimeSpan span);
    CFileTimeSpan &operator-=(CFileTimeSpan span);

    CFileTimeSpan operator+(CFileTimeSpan span);
    CFileTimeSpan operator-(CFileTimeSpan span);

    bool operator==(CFileTimeSpan span);
    bool operator!=(CFileTimeSpan span);
    bool operator<(CFileTimeSpan span);
    bool operator>(CFileTimeSpan span);
    bool operator<=(CFileTimeSpan span);
    bool operator>=(CFileTimeSpan span);

    LONGLONG GetTimeSpan();
    void SetTimeSpan(LONGLONG nSpan);

    LONGLONG GetTotalMillisecond();
    LONGLONG GetTotalSecond();

protected:
    LONGLONG m_nSpan;
};

class CFileTime : public FILETIME {
public:
    CFileTime();
    CFileTime(const FILETIME &ft);
    CFileTime(ULONGLONG nTime);

    //static CFileTime FromSystemTime(SYSTEMTIME &st);

    static CFileTime GetCurrentFileTime();

    SYSTEMTIME ToSystemTime();

    CFileTime &operator=(const FILETIME &ft);

    CFileTime &operator+=(CFileTimeSpan span);
    CFileTime &operator-=(CFileTimeSpan span);

    CFileTime operator+(CFileTimeSpan span);
    CFileTime operator-(CFileTimeSpan span);
    CFileTimeSpan operator-(CFileTime ft);

    bool operator==(CFileTime ft);
    bool operator!=(CFileTime ft);
    bool operator<(CFileTime ft);
    bool operator>(CFileTime ft);
    bool operator<=(CFileTime ft);
    bool operator>=(CFileTime ft);

    ULONGLONG GetTime();
    void SetTime(ULONGLONG nTime);

    CFileTime UTCToLocal();
    CFileTime LocalToUTC();

    std::string Format();
    std::string FormatTZ();

    static const ULONGLONG Millisecond = 10000;
    static const ULONGLONG Second = Millisecond * static_cast<ULONGLONG>(1000);
    static const ULONGLONG Minute = Second * static_cast<ULONGLONG>(60);
    static const ULONGLONG Hour = Minute * static_cast<ULONGLONG>(60);
    static const ULONGLONG Day = Hour * static_cast<ULONGLONG>(24);
    static const ULONGLONG Week = Day * static_cast<ULONGLONG>(7);
};

}

#endif
