#ifndef SINDY_RUN_TIME_H
#define SINDY_RUN_TIME_H

#include "../SindyGlobal.h"
#include <chrono>

#ifndef INCLUDE_SINDY_GLOBAL
#define SINDY_API
#include <map>
#include <vector>
#include <string>
#define ConverToMilliseconds(durationX) std::chrono::duration_cast<std::chrono::milliseconds>((durationX))
#endif

namespace sindy
{
	using TimePoint = std::chrono::steady_clock::time_point;
	using DurationTime = std::chrono::steady_clock::duration;

	class SINDY_API RunTime
	{
		TimePoint m_firstTime;
		TimePoint m_lastTime;
		std::map<std::string, DurationTime> m_mapLog2Time;
		std::vector<std::string> m_arrLog;

		void _addRunTime(const std::string& strCatalog, DurationTime offset);
		std::string _catalog(const std::string& strInput = "");
	public:
		RunTime();
		void reset();

		// 获取上一个log到当前log流逝的时间
		long addTimePoint(const std::string& strLog = "");
		std::string str(const std::string& strTitle);
		void write(const std::string& strFileFullPath, const std::string& strTitle = "");
	};

} // namespace sindy

#endif !SINDY_RUN_TIME_H
