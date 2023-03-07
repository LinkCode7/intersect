#include "./Testing/TestRange2dCase.h"
#include "./Testing/TestRangeBound.h"
#include "./Testing/TestBoostRtree.h"

int main()
{
	using namespace Sindy;
	TestRange2dCase::entry();

	//TestPerformance::testAccuracy(MAKE_SINDY_PATH("TestData.db3"));
	TestPerformance::entry(MAKE_SINDY_PATH("TestData.db3"), MAKE_SINDY_PATH("TestResult.log"));

	TestBoostRtree::entry(MAKE_SINDY_PATH("TestData.db3"), MAKE_SINDY_PATH("TestResult.log"));
	return 0;
}
