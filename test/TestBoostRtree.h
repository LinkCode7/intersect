#ifndef TEST_BOOST_RTREE_H
#define TEST_BOOST_RTREE_H
#include "../SindyGlobal.h"

namespace Sindy
{
	class TestLineData;
}

namespace TestBoostRtree
{
	void entry(const PString& strDbPath, const PString& strLogPath);

	void queryOfpack(const std::vector<Sindy::TestLineData*>& arrLineData, const PString& strDbPath, const PString& strLogPath);
	void quickStart(const std::vector<Sindy::TestLineData*>& arrLineData, const PString& strDbPath, const PString& strLogPath);
}

#endif // TEST_BOOST_RTREE_H
