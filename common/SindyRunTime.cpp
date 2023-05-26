#include "SindyRunTime.h"

#include <chrono>
#include <fstream>
#include <sstream>

using namespace std::chrono;

namespace sindy
{
RunTime::RunTime() : m_firstTime(steady_clock::now()), m_lastTime(m_firstTime)
{
}

long RunTime::addTimePoint(const std::string& strLog)
{
    TimePoint curTime = steady_clock::now();
    auto      offset  = curTime - m_lastTime;
    m_lastTime        = curTime;

    _addRunTime(strLog, offset);
    long result = ConverToMilliseconds(offset).count();
    return result;
}

void RunTime::reset()
{
    m_mapLog2Time.clear();
    m_arrLog.clear();
    m_firstTime = steady_clock::now();
    m_lastTime  = m_firstTime;
}

void RunTime::_addRunTime(const std::string& strCatalog, DurationTime offset)
{
    auto iter = m_mapLog2Time.find(strCatalog);
    if (iter != m_mapLog2Time.end())
    {
        iter->second += offset;
    }
    else
    {
        m_mapLog2Time[strCatalog] = offset;
        m_arrLog.emplace_back(strCatalog);
    }
}

std::string RunTime::_catalog(const std::string& strInput)
{
    if (m_arrLog.empty())
        addTimePoint();

    std::string strMsg(strInput);
    strMsg += "\n";

    std::ostringstream oss;

    if (m_arrLog.size() == 1)
    {
        auto iter = m_mapLog2Time.find(*(m_arrLog.begin()));
        if (iter != m_mapLog2Time.end())
        {
            oss << *(m_arrLog.begin()) << ": " << ConverToMilliseconds(iter->second).count() << "ms" << std::endl;
            strMsg = oss.str();
        }
    }
    else
    {
        for (const auto& strLog : m_arrLog)
        {
            auto iter = m_mapLog2Time.find(strLog);
            if (iter == m_mapLog2Time.end())
                continue;

            oss << strLog << ": " << ConverToMilliseconds(iter->second).count() << "ms" << std::endl;
        }
        strMsg = oss.str();
    }
    return strMsg;
}

std::string RunTime::str(const std::string& strTitle)
{
    std::stringstream log;

    TimePoint curTime = steady_clock::now();
    long      all     = ConverToMilliseconds(curTime - m_firstTime).count();

    if (m_arrLog.size() == 1 && m_arrLog[0].empty())  // 只有一个标签
    {
        auto iter = m_mapLog2Time.find(m_arrLog[0]);
        if (iter != m_mapLog2Time.end())
        {
            all = ConverToMilliseconds(iter->second).count();
            log << std::endl << strTitle << " 总计：" << all << "ms" << std::endl;
            return log.str();
        }
    }

    log << std::endl << strTitle << " 总计：" << all << "ms" << std::endl;
    log << _catalog() << std::endl;
    return log.str();
}

void RunTime::write(const std::string& strFileFullPath, const std::string& strTitle)
{
    if (m_arrLog.empty())
        addTimePoint();

    //// 将全局locale设为本地语言
    // locale& loc = locale::global(locale(locale(), "", LC_CTYPE));
    std::ofstream ofs(strFileFullPath, std::ios::app);
    //// 将全局locale恢复
    // locale::global(loc);

    ofs << str(strTitle);
}

}  // namespace sindy