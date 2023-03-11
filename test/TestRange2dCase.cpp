#include "TestRange2dCase.h"
#include <memory>

void TestRange2dCase::entry()
{
	testIntersect();
}

int TestRange2dCase::check(const std::vector<BoxInfo>& arrBox, const std::vector<std::string>& expect, double tol)
{
	std::vector<std::shared_ptr<LineData2>> arrLineData;
	for (const auto& box : arrBox) {
		arrLineData.push_back(std::make_shared<LineData2>(box.m_ptMin, box.m_ptMax, box.m_strHandle));
	}

	Sindy::Range2d range;
	for (const auto& pData : arrLineData)
	{
		range.setItem(pData.get(), true, tol * 0.5);
	}

	std::vector<Sindy::RangeItem*> vecItem;
	range.getIntersectItem(vecItem);

	std::set<std::string> result;

	std::vector<Sindy::RangeItem*>::iterator iter = vecItem.begin();
	for (; iter != vecItem.end(); ++iter)
	{
		Sindy::RangeItem* pItem = *iter;
		auto pSrcLineData = static_cast<LineData2*>(pItem->m_ipItem);

		auto it = pItem->m_pItems->m_items.begin();
		for (; it != pItem->m_pItems->m_items.end(); ++it)
		{
			Sindy::RangeItem* pIntersectItem = *it;
			auto pDesLineData = static_cast<LineData2*>(pIntersectItem->m_ipItem);

			std::string relation;
			if (std::atoi(pSrcLineData->m_strId.c_str()) < std::atoi(pDesLineData->m_strId.c_str()))
				relation = pSrcLineData->m_strId + "-" + pDesLineData->m_strId;
			else
				relation = pDesLineData->m_strId + "-" + pSrcLineData->m_strId;
			result.emplace(relation);
		}
	}

	if (result.size() != expect.size())
		return 0;

	for (const auto& str : expect)
	{
		if (result.find(str) == result.end())
			return 0;
	}

	return 1;
}

void testt();

void TestRange2dCase::testIntersect()
{
	testt();

	std::vector<BoxInfo> arr = { {{0,0}, {1,1}, "1"} };

	int flag = 1;
	flag &= check({ {{0,0},{1,1},"1"}, {{1,1},{2,2},"2"} }, { }, 0.0);
	flag &= check({ {{0,0},{1,1},"1"}, {{1,1},{2,2},"2"} }, { "1-2" }, 0.001);
	flag &= check({ {{0,0},{1,1},"1"}, {{0.99,0.99},{2,2},"2"} }, { "1-2" }, 0.0);

	flag &= check({ {{10,20},{15,30},"1"}, {{3,5},{10.001,20.001},"2"} }, { "1-2" }, 0.0);
	flag &= check({ {{10,20},{15,30},"1"}, {{3,5},{10,20},"2"} }, { "1-2" }, 0.001);
	flag &= check({ {{3,5},{10,20},"2"}, {{10,20},{15,30},"1"} }, { }, 0.0);
	flag &= check({ {{10,20},{15,30},"1"}, {{3,5},{10,20},"2"} }, { "1-2" }, 0.0);
}

void testt()
{

}