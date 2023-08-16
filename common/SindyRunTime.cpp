#include "SindyRunTime.h"

#include <chrono>
#include <fstream>
#include <sstream>

using namespace std::chrono;

namespace sindy
{
RunTime::RunTime() : _firstTime(steady_clock::now()), _lastTime(_firstTime)
{
}

long RunTime::addTimePoint(std::string const& log)
{
    TimePoint curTime = steady_clock::now();
    auto      offset  = curTime - _lastTime;
    _lastTime         = curTime;

    _addRunTime(log, offset);
    long result = ConverToMilliseconds(offset).count();
    return result;
}

void RunTime::reset()
{
    _mapLog2Time.clear();
    _arrLog.clear();
    _firstTime = steady_clock::now();
    _lastTime  = _firstTime;
}

void RunTime::_addRunTime(std::string const& catalog, DurationTime offset)
{
    auto iter = _mapLog2Time.find(catalog);
    if (iter != _mapLog2Time.end())
    {
        iter->second += offset;
    }
    else
    {
        _mapLog2Time[catalog] = offset;
        _arrLog.emplace_back(catalog);
    }
}

std::string RunTime::str(std::string const& title) const
{
    TimePoint curTime = steady_clock::now();
    long      all     = ConverToMilliseconds(curTime - _firstTime).count();

    std::stringstream ss;
    if (title.empty())
        ss << std::endl << "total: " << all << "ms" << std::endl;
    else
        ss << std::endl << title << " total: " << all << "ms" << std::endl;

    for (const auto& log : _arrLog)
    {
        auto iter = _mapLog2Time.find(log);
        if (iter == _mapLog2Time.end())
            continue;

        ss << log << ": " << ConverToMilliseconds(iter->second).count() << "ms" << std::endl;
    }

    return ss.str();
}

void RunTime::console(std::string const& title) const
{
    std::cout << str();
}

void RunTime::write(std::string const& fullPath, std::string const& title) const
{
    // std::locale&  loc = std::locale::global(std::locale(std::locale(), "", LC_CTYPE)); // 设全局locale为本地语言
    std::ofstream ofs(fullPath, std::ios::app);
    // std::locale::global(loc); // 恢复全局locale

    ofs << str(title);
}

} // namespace sindy