#include "./test/TestBoostRtree.h"
#include "./test/TestRange2dCase.h"
#include "./test/TestRangeBound.h"

int main()
{
    TestRange2dCase::entry();

    // TestPerformance::testAccuracy(MAKE_SINDY_PATH("TestData.db3"));

    TestPerformance::entry(MAKE_SINDY_PATH("TestData.db3"), MAKE_SINDY_PATH("TestResult.log"));

    TestBoostRtree::entry(MAKE_SINDY_PATH("TestData.db3"), MAKE_SINDY_PATH("TestResult.log"));
    return 0;
}
