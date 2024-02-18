#ifndef BASE_FILE_FILE_PATH_H_
#define BASE_FILE_FILE_PATH_H_

#include <string>
#include <limits>
namespace nbase
{

#if defined(OS_POSIX)
typedef std::string PathString;
#elif defined(OS_WIN)
typedef std::wstring PathString;
#endif

typedef PathString::value_type PathChar;

} // namespace nbase

#endif // BASE_FILE_FILE_PATH_H_
