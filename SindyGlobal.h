#ifndef SINDY_GLOBAL_H
#define SINDY_GLOBAL_H

#define INCLUDE_SINDY_GLOBAL

#define SINDY_API

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>

//#define SINDY_UNICODE
#ifdef SINDY_UNICODE
using PString = std::wstring;
#else
using PString = std::string;
#endif

#define ConverToMilliseconds(durationX) std::chrono::duration_cast<std::chrono::milliseconds>((durationX))


#ifndef _T

#ifdef SINDY_UNICODE
#define _T(str) L#str
#else
#define _T(str) str
#endif

#endif

#define SINDY_DB_PATH "E:\\Sindy\\intersect\\TestData.db3"
#define SINDY_LOG_PATH _T("E:\\Sindy\\intersect\\TestResult.log")

#endif
