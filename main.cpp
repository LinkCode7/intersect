#include "./Testing/TestRangeBound.h"

#include "SindyGlobal.h"
#include "Common/SindyRunTime.h"

#include <thread>
#include <codecvt>

#include <chrono>
#include <sstream>
#include <fstream>

using namespace std::chrono;

int main()
{
	using namespace Sindy;
	//TestPerformance::testAccuracy(SINDY_DB_PATH);
	TestPerformance::entry(SINDY_DB_PATH, SINDY_LOG_PATH);

	DurationTime first = steady_clock::now().time_since_epoch();
	DurationTime now = steady_clock::now().time_since_epoch();

	auto diff = now - first;
	long result = ConverToMilliseconds(diff).count();
	return 0;
}