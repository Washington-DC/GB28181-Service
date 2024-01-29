#include "pch.h"
#include "FileTime.h"

namespace toolkit {
CFileTimeSpan::CFileTimeSpan()
    : m_nSpan(0) {}

CFileTimeSpan::CFileTimeSpan(const CFileTimeSpan &span)
    : m_nSpan(span.m_nSpan) {}

CFileTimeSpan::CFileTimeSpan(LONGLONG nSpan)
    : m_nSpan(nSpan) {}

CFileTimeSpan &CFileTimeSpan::operator=(const CFileTimeSpan &span) {
    m_nSpan = span.m_nSpan;

    return (*this);
}

CFileTimeSpan &CFileTimeSpan::operator+=(CFileTimeSpan span) {
    m_nSpan += span.m_nSpan;

    return (*this);
}

CFileTimeSpan &CFileTimeSpan::operator-=(CFileTimeSpan span) {
    m_nSpan -= span.m_nSpan;

    return (*this);
}

CFileTimeSpan CFileTimeSpan::operator+(CFileTimeSpan span) {
    return (CFileTimeSpan(m_nSpan + span.m_nSpan));
}

CFileTimeSpan CFileTimeSpan::operator-(CFileTimeSpan span) {
    return (CFileTimeSpan(m_nSpan - span.m_nSpan));
}

bool CFileTimeSpan::operator==(CFileTimeSpan span) {
    return (m_nSpan == span.m_nSpan);
}

bool CFileTimeSpan::operator!=(CFileTimeSpan span) {
    return (m_nSpan != span.m_nSpan);
}

bool CFileTimeSpan::operator<(CFileTimeSpan span) {
    return (m_nSpan < span.m_nSpan);
}

bool CFileTimeSpan::operator>(CFileTimeSpan span) {
    return (m_nSpan > span.m_nSpan);
}

bool CFileTimeSpan::operator<=(CFileTimeSpan span) {
    return (m_nSpan <= span.m_nSpan);
}

bool CFileTimeSpan::operator>=(CFileTimeSpan span) {
    return (m_nSpan >= span.m_nSpan);
}

LONGLONG CFileTimeSpan::GetTimeSpan() {
    return (m_nSpan);
}

void CFileTimeSpan::SetTimeSpan(LONGLONG nSpan) {
    m_nSpan = nSpan;
}

LONGLONG CFileTimeSpan::GetTotalMillisecond() {
    return (m_nSpan/10/1000);
}

LONGLONG CFileTimeSpan::GetTotalSecond() {
    return (GetTotalMillisecond() / 1000);
}

//============================================================================

CFileTime::CFileTime() {
    dwLowDateTime = 0;
    dwHighDateTime = 0;
}

CFileTime::CFileTime(const FILETIME &ft) {
    dwLowDateTime = ft.dwLowDateTime;
    dwHighDateTime = ft.dwHighDateTime;
}

CFileTime::CFileTime(ULONGLONG nTime) {
    dwLowDateTime = DWORD(nTime);
    dwHighDateTime = DWORD(nTime >> 32);
}

//CFileTime CFileTime::FromSystemTime(SYSTEMTIME &st) {
//    FILETIME ft;
//    SystemTimeToFileTime(&st, &ft);
//    return CFileTime(ft);
//}

CFileTime &CFileTime::operator=(const FILETIME &ft) {
    dwLowDateTime = ft.dwLowDateTime;
    dwHighDateTime = ft.dwHighDateTime;

    return (*this);
}

CFileTime CFileTime::GetCurrentFileTime() {
    CFileTime ft;
#ifdef _WIN32
    GetSystemTimeAsFileTime(&ft);
#else
    ft = GetCurrentFileTime();
#endif
    return ft;
}

SYSTEMTIME CFileTime::ToSystemTime() {
    SYSTEMTIME st;
    FileTimeToSystemTime(this, &st);
    return st;
}

CFileTime &CFileTime::operator+=(CFileTimeSpan span) {
    SetTime(GetTime() + span.GetTimeSpan());

    return (*this);
}

CFileTime &CFileTime::operator-=(CFileTimeSpan span) {
    SetTime(GetTime() - span.GetTimeSpan());

    return (*this);
}

CFileTime CFileTime::operator+(CFileTimeSpan span) {
    return (CFileTime(GetTime() + span.GetTimeSpan()));
}

CFileTime CFileTime::operator-(CFileTimeSpan span) {
    return (CFileTime(GetTime() - span.GetTimeSpan()));
}

CFileTimeSpan CFileTime::operator-(CFileTime ft) {
    return (CFileTimeSpan(GetTime() - ft.GetTime()));
}

bool CFileTime::operator==(CFileTime ft) {
    return (GetTime() == ft.GetTime());
}

bool CFileTime::operator!=(CFileTime ft) {
    return (GetTime() != ft.GetTime());
}

bool CFileTime::operator<(CFileTime ft) {
    return (GetTime() < ft.GetTime());
}

bool CFileTime::operator>(CFileTime ft) {
    return (GetTime() > ft.GetTime());
}

bool CFileTime::operator<=(CFileTime ft) {
    return (GetTime() <= ft.GetTime());
}

bool CFileTime::operator>=(CFileTime ft) {
    return (GetTime() >= ft.GetTime());
}

ULONGLONG CFileTime::GetTime() {
    return ((ULONGLONG(dwHighDateTime) << 32) | dwLowDateTime);
}

void CFileTime::SetTime(ULONGLONG nTime) {
    dwLowDateTime = DWORD(nTime);
    dwHighDateTime = DWORD(nTime >> 32);
}

CFileTime CFileTime::UTCToLocal() {
    CFileTime ftLocal;

    FileTimeToLocalFileTime(this, &ftLocal);

    return (ftLocal);
}

CFileTime CFileTime::LocalToUTC() {
    CFileTime ftUTC;

    LocalFileTimeToFileTime(this, &ftUTC);

    return (ftUTC);
}

std::string CFileTime::Format() {
    auto st = ToSystemTime();
    char sz[50] = { 0 };
    sprintf(
        sz, "%04d-%02d-%02d %02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
        st.wMilliseconds);
    return std::string(sz);
}


std::string CFileTime::FormatTZ() {
    auto st = ToSystemTime();
    char sz[50] = { 0 };
    sprintf(
        sz, "%04d-%02d-%02dT%02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    return std::string(sz);
}

}

