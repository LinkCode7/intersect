#include "TestRange2dCase.h"
#include <memory>
#include <cassert>

void TestRange2dCase::entry()
{
	testBoundSort();
	testIntersect();
}

std::vector<std::shared_ptr<TestRange2dCase::LineData2>> TestRange2dCase::_makeLineData(const std::vector<BoxInfo>& arrBox)
{
	std::vector<std::shared_ptr<LineData2>> arrLineData;
	for (const auto& box : arrBox) {
		arrLineData.push_back(std::make_shared<LineData2>(box.m_ptMin, box.m_ptMax, box.m_strHandle));
	}
	return arrLineData;
}

int TestRange2dCase::check(const std::vector<BoxInfo>& arrBox, const std::vector<std::string>& expect, double tol)
{
	auto arrLineData = _makeLineData(arrBox);

	Sindy::Range2d range;
	range.setRangeItems(arrLineData, true, tol * 0.5);

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

void TestRange2dCase::testIntersect()
{
	std::vector<BoxInfo> arr = { {{0,0}, {1,1}, "1"} };

	int flag = 1;
	flag &= check({ {{0,0},{1,1},"1"}, {{1,1},{2,2},"2"} }, { "1-2" }, 0.0);
	flag &= check({ {{0,0},{1,1},"1"}, {{1,1},{2,2},"2"} }, { "1-2" }, 0.001);
	flag &= check({ {{0,0},{1,1},"1"}, {{0.99,0.99},{2,2},"2"} }, { "1-2" }, 0.0);

	flag &= check({ {{10,20},{15,30},"1"}, {{3,5},{10.001,20.001},"2"} }, { "1-2" }, 0.0);
	flag &= check({ {{10,20},{15,30},"1"}, {{3,5},{10,20},"2"} }, { "1-2" }, 0.001);
	flag &= check({ {{3,5},{10,20},"2"}, {{10,20},{15,30},"1"} }, { "1-2" }, 0.0);
	flag &= check({ {{10,20},{15,30},"1"}, {{3,5},{10,20},"2"} }, { "1-2" }, 0.0);

	assert(flag == 1);
}

void TestRange2dCase::testBoundSort()
{
	using namespace Sindy;

	auto checkSort = [](const std::vector<BoxInfo>& arrBox, const std::vector<double>& expect) -> int {
		Range2d range;
		auto arrLineData = _makeLineData(arrBox);
		range.setRangeItems(arrLineData, true, 0.0);
		std::vector<double> result = range.testSortBox();

		auto size = result.size();
		if (size != expect.size())
			return 0;

		for (auto i = 0; i < size; ++i)
		{
			if (std::to_string(result[i]) != std::to_string(expect[i]))
				return 0;
		}
		return 1;
	};

	/*
	* 本测试用例只关注包围盒的X值，主要测试两个包围盒的最小点、最大点的X值相等时，包围盒item的顺序
	* 如：box1.min = {0,0};box1.max = {1,0};box2.min = {1,0};box2.max = {2,2};
	* 我们期望靠右的box排在前面，即：box2.min.x,box1.max.x
	* 返回值：整数部分为box的ID，小数部分只有一位，十分位0代表box最小点，1代表box最大点
	*/

	// 注意期望值不是{ box1.min.x, box1.max.x, box2.min.x, box2.max.x }，即{ 1.0, 1.1, 2.0, 2.1 }
	std::vector<double> expect = { 1.0, 2.0, 1.1, 2.1 };

	int flag = 1;
	flag &= checkSort({ {{0,0},{1,0},"1"}, {{1,0},{2,2},"2"} }, expect);
	flag &= checkSort({ {{0,0},{1,1},"1"}, {{1,1},{2,2},"2"} }, expect);
	flag &= checkSort({ {{-1,-1},{0,0},"1"}, {{0,0},{1,1},"2"} }, expect);
	flag &= checkSort({ {{-1,-1},{0,0},"1"}, {{0,0},{1,1},"2"} }, expect);

	flag &= checkSort({ {{0,0},{1,1},"1"}, {{0.99,0.99},{2,2},"2"} }, expect);
	flag &= checkSort({ {{-5,-5},{-3.14,-3.14},"1"}, {{-3.14,0},{1,1},"2"} }, expect);

	// 特殊值
	flag &= checkSort({ {{0,0},{0,0},"1"}, {{0,0},{2,2},"2"} }, expect);
	flag &= checkSort({ {{0.618,0.618},{0.618,0.618},"1"}, {{0.618,0.618},{1.618,1.618},"2"} }, expect);
	flag &= checkSort({ {{30.9,30.9},{30.9,30.9},"1"}, {{30.9,30.9},{30.9,30.9},"2"} }, expect);

	assert(flag == 1);
}