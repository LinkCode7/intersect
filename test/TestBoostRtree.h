#ifndef TEST_BOOST_RTREE_H
#define TEST_BOOST_RTREE_H
#include "../SindyGlobal.h"

namespace sindy
{
class TestLineData;
}

namespace TestBoostRtree
{
void entry(const PString& strDbPath, const PString& strLogPath);

void queryOfPacking(const std::vector<sindy::TestLineData*>& arrLineData, const PString& strDbPath, const PString& strLogPath);
void quickStart(const std::vector<sindy::TestLineData*>& arrLineData, const PString& strDbPath, const PString& strLogPath);
} // namespace TestBoostRtree

#endif // TEST_BOOST_RTREE_H
