#ifndef SINDY_GLOBAL_H
#define SINDY_GLOBAL_H

#define SINDY_API

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>


#ifdef UNICODE
using PString = std::wstring;
#else
using PString = std::string;
#endif // UNICODE

#define ConverToMilliseconds(durationX) std::chrono::duration_cast<std::chrono::milliseconds>((durationX))

#define SINDY_DB_PATH "E:\\Sindy\\intersect\\TestData.db"
#define SINDY_LOG_PATH "E:\\Sindy\\intersect\\TestResult.log"

#endif
