#ifndef SINDY_RUN_TIME_H
#define SINDY_RUN_TIME_H

#include <chrono>

#include "../SindyGlobal.h"

#ifndef INCLUDE_SINDY_GLOBAL
    #define SINDY_API
    #include <map>
    #include <string>
    #include <vector>
    #define ConverToMilliseconds(durationX) std::chrono::duration_cast<std::chrono::milliseconds>((durationX))
#endif

namespace sindy
{
using TimePoint    = std::chrono::steady_clock::time_point;
using DurationTime = std::chrono::steady_clock::duration;

class SINDY_API RunTime
{
    TimePoint                           _firstTime;
    TimePoint                           _lastTime;
    std::map<std::string, DurationTime> _mapLog2Time;
    std::vector<std::string>            _arrLog;

    void _addRunTime(std::string const & catalog, DurationTime offset);

public:
    RunTime();
    void reset();

    // 获取上一个log到当前log流逝的时间
    long addTimePoint(std::string const & log = "");

    std::string str(std::string const & title = "") const;
    void        console(std::string const & title = "") const;
    void        write(std::string const & fullPath, std::string const & title = "") const;
};

} // namespace sindy

#endif !SINDY_RUN_TIME_H
