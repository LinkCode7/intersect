#include "SindyRunTime.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

using namespace std::chrono;

namespace sindy
{
RunTime::RunTime(std::string const& title)
{
    reset(title);
}

void RunTime::reset(std::string const& title)
{
    if (!title.empty())
        std::cout << title << std::endl;

    _mapLog2Time.clear();
    _arrLog.clear();
    _firstTime = steady_clock::now();
    _lastTime  = _firstTime;
}

long RunTime::addTimePoint(std::string const& log)
{
    TimePoint curTime = steady_clock::now();
    auto      offset  = curTime - _lastTime;
    _lastTime         = curTime;

    auto iter = _mapLog2Time.find(log);
    if (iter != _mapLog2Time.end())
    {
        iter->second += offset;
    }
    else
    {
        _mapLog2Time[log] = offset;
        _arrLog.emplace_back(log);
    }

    long result = ConverToMilliseconds(offset).count();
    return result;
}

std::string RunTime::str(std::string const& prefix) const
{
    TimePoint curTime = steady_clock::now();
    long      all     = ConverToMilliseconds(curTime - _firstTime).count();

    std::stringstream ss;
    if (prefix.empty())
        ss << "total: " << all << "ms" << std::endl;
    else
        ss << prefix << " total: " << all << "ms" << std::endl;

    for (const auto& log : _arrLog)
    {
        auto iter = _mapLog2Time.find(log);
        if (iter == _mapLog2Time.end())
            continue;

        ss << "   " << log << ": " << ConverToMilliseconds(iter->second).count() << "ms" << std::endl;
    }

    return ss.str();
}

void RunTime::console(std::string const& prefix) const
{
    std::cout << str();
}

void RunTime::write(std::string const& fullPath, std::string const& prefix) const
{
    // std::locale&  loc = std::locale::global(std::locale(std::locale(), "", LC_CTYPE)); // 设全局locale为本地语言
    std::ofstream ofs(fullPath, std::ios::app);
    // std::locale::global(loc); // 恢复全局locale

    ofs << str(prefix);
}

void RunTime::testCase()
{
    // The first way
    RunTime time("step");

    std::cout << "step1..." << std::endl;
    time.addTimePoint("step1");

    std::cout << "step2..." << std::endl;
    time.addTimePoint("step2");

    time.console();

    // The second way
    auto f1 = [&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        RunTime::instance().addTimePoint(".f1");
    };

    auto f2 = [&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        RunTime::instance().addTimePoint("f2");
        f1();
    };

    auto f = [&]() {
        f1();
        // f2();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    RunTime::instance().reset("call function");
    f();
    RunTime::instance().addTimePoint("f");

    RunTime::instance().console("call function");
}

} // namespace sindy