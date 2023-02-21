#include "./Testing/TestRangeBound.h"

int main()
{
	using namespace Sindy;
	//TestPerformance::testAccuracy(MAKE_SINDY_PATH("TestData.db3"));
	TestPerformance::entry(MAKE_SINDY_PATH("TestData.db3"), MAKE_SINDY_PATH("TestResult.log"));
	return 0;
}
