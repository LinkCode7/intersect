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

#define MAKE_SINDY_PATH(name) THIS_PROJECT_SOURCE_DIR##"//"##name

#endif
