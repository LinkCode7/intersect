#include "TestRangeBound.h"

#include "../SQLite/SindySQLite.h"

#include "../Common/SindyRunTime.h"
#include "../Common/StringUtil.h"
#include "../Common/template.h"
#include <sstream>
#include <codecvt>

void TestPerformance::entry(const PString& strDbPath, const PString& strLogPath)
{
	using namespace Sindy;

	std::vector<TestLineData*> vecLineData;
	unSerializePoints(strDbPath, vecLineData);

	testFor(vecLineData, strLogPath);
	testOutOf(vecLineData, strLogPath);
	testRangeBound(vecLineData, strLogPath);

	for (auto& info : vecLineData)
		delete info;
}

void TestPerformance::testFor(const std::vector<TestLineData*>& vecLineData, const PString& strLogPath)
{
	using namespace Sindy;
	RunTime time;

	int size = static_cast<int>(vecLineData.size());
	for (int i = 0; i < size; ++i)
	{
		TestLineData* pSrcLineData = vecLineData[i];

		//for (int j = 0; j < size; ++j) // 245969ms
		for (int j = i + 1; j < size; ++j) // 100360ms
		{
			TestLineData* pDesLineData = vecLineData[j];

			bool isLink = false;
			if (isSamePt(pSrcLineData->m_ptBegin, pDesLineData->m_ptBegin))
				isLink = true;
			else if (isSamePt(pSrcLineData->m_ptBegin, pDesLineData->m_ptEnd))
				isLink = true;
			else if (isSamePt(pSrcLineData->m_ptEnd, pDesLineData->m_ptBegin))
				isLink = true;
			else if (isSamePt(pSrcLineData->m_ptEnd, pDesLineData->m_ptEnd))
				isLink = true;
		}
	}

	time.addTimePoint("");

	std::ostringstream oss;
	oss << "对" << vecLineData.size() << "个实体求交 -> 算法1：暴力遍历";
	std::string strText = oss.str();

	time.write(strLogPath, strText);
}

void TestPerformance::testOutOf(const std::vector<TestLineData*>& vecLineData, const PString& strLogPath)
{
	using namespace Sindy;
	RunTime time;

	int size = static_cast<int>(vecLineData.size());
	for (int i = 0; i < size; ++i)
	{
		TestLineData* pSrcLineData = vecLineData[i];

		for (int j = i + 1; j < size; ++j) // 100360ms
		{
			TestLineData* pDesLineData = vecLineData[j];

			// 外包排斥
			if (pSrcLineData->m_extents.outExtents(pDesLineData->m_extents))
				continue;

			bool isLink = false;
			if (isSamePt(pSrcLineData->m_ptBegin, pDesLineData->m_ptBegin))
				isLink = true;
			else if (isSamePt(pSrcLineData->m_ptBegin, pDesLineData->m_ptEnd))
				isLink = true;
			else if (isSamePt(pSrcLineData->m_ptEnd, pDesLineData->m_ptBegin))
				isLink = true;
			else if (isSamePt(pSrcLineData->m_ptEnd, pDesLineData->m_ptEnd))
				isLink = true;
		}
	}

	time.addTimePoint("");

	std::ostringstream oss;
	oss << "对" << vecLineData.size() << "个实体求交 -> 算法2：外包排斥";
	std::string strText = oss.str();

	time.write(strLogPath, strText);
}

void TestPerformance::testRangeBound(const std::vector<TestLineData*>& vecLineData, const PString& strLogPath)
{
	using namespace Sindy;
	RunTime time;

	Range2d range;
	for (const auto& pData : vecLineData)
	{
		range.SetItems(pData, true, 100);
	}

	std::vector<RangeItem*> vecItem;
	range.GetIntersectItems(vecItem);

	std::vector<RangeItem*>::iterator iter = vecItem.begin();
	for (; iter != vecItem.end(); ++iter) // 771ms
	{
		RangeItem* pItem = *iter;
		TestLineData* pSrcLineData = static_cast<TestLineData*>(pItem->m_ipItem);

		std::vector<RangeItem*>::iterator it = pItem->m_pItems->m_items.begin();
		for (; it != pItem->m_pItems->m_items.end(); ++it)
		{
			RangeItem* pIntersectItem = *it;
			TestLineData* pDesLineData = static_cast<TestLineData*>(pIntersectItem->m_ipItem);

			bool isLink = false;
			if (isSamePt(pSrcLineData->m_ptBegin, pDesLineData->m_ptBegin))
				isLink = true;
			else if (isSamePt(pSrcLineData->m_ptBegin, pDesLineData->m_ptEnd))
				isLink = true;
			else if (isSamePt(pSrcLineData->m_ptEnd, pDesLineData->m_ptBegin))
				isLink = true;
			else if (isSamePt(pSrcLineData->m_ptEnd, pDesLineData->m_ptEnd))
				isLink = true;
		}
	}

	time.addTimePoint("");

	std::ostringstream oss;
	oss << "对" << vecLineData.size() << "个实体求交 -> 算法3：Range2d";
	std::string strText = oss.str();

	// LOG_FILE_PATH
	time.write(strLogPath, strText);
}

void TestPerformance::unSerializePoints(const PString& strDbPath, std::vector<TestLineData*>& vecLineData)
{
	// TestData.Line
	Sindy::SQLite database(strDbPath);

	std::ostringstream oss;
	oss << "select * from " << table_name;
	std::string strSql = oss.str();

	database.beginTransaction();
	database.prepare(strSql);

	while (SQLITE_ROW == database.step())
	{
		TestLineData* pLineData = new TestLineData;
		database.getValueText(handle, pLineData->m_strId);

		database.getValueDouble(bulge, pLineData->m_dBulge);
		database.getValueDouble(from_x, pLineData->m_ptBegin.x);
		database.getValueDouble(from_y, pLineData->m_ptBegin.y);
		database.getValueDouble(to_x, pLineData->m_ptEnd.x);
		database.getValueDouble(to_y, pLineData->m_ptEnd.y);

		double dMinX = 0.0;
		database.getValueDouble(min_x, dMinX);
		double dMinY = 0.0;
		database.getValueDouble(min_y, dMinY);
		double dMaxX = 0.0;
		database.getValueDouble(max_x, dMaxX);
		double dMaxY = 0.0;
		database.getValueDouble(max_y, dMaxY);

		pLineData->m_extents.reset(Point3d(dMinX, dMinY, 0.0), Point3d(dMaxX, dMaxY, 0.0));

		vecLineData.push_back(pLineData);
	}
}

void TestPerformance::testAccuracy(const PString& strDbPath)
{
	using namespace Sindy;
	std::vector<TestLineData*> vecLineData;
	unSerializePoints(strDbPath, vecLineData);

	std::map<std::string, std::set<std::string>*> mapLine2Links1, mapLine2Links2;
	{
		Range2d range;
		for (const auto& pData : vecLineData)
			range.SetItems(pData, true, 100);

		std::vector<RangeItem*> vecItem;
		range.GetIntersectItems(vecItem);

		std::vector<RangeItem*>::iterator iter = vecItem.begin();
		for (; iter != vecItem.end(); ++iter) // 771ms
		{
			RangeItem* pItem = *iter;
			TestLineData* pSrcLineData = static_cast<TestLineData*>(pItem->m_ipItem);

			std::vector<RangeItem*>::iterator it = pItem->m_pItems->m_items.begin();
			for (; it != pItem->m_pItems->m_items.end(); ++it)
			{
				RangeItem* pIntersectItem = *it;
				TestLineData* pDesLineData = static_cast<TestLineData*>(pIntersectItem->m_ipItem);

				bool isLink = false;
				if (isSamePt(pSrcLineData->m_ptBegin, pDesLineData->m_ptBegin))
					isLink = true;
				else if (isSamePt(pSrcLineData->m_ptBegin, pDesLineData->m_ptEnd))
					isLink = true;
				else if (isSamePt(pSrcLineData->m_ptEnd, pDesLineData->m_ptBegin))
					isLink = true;
				else if (isSamePt(pSrcLineData->m_ptEnd, pDesLineData->m_ptEnd))
					isLink = true;

				if (!isLink)
					continue;

				_testAccuracy(mapLine2Links1, pSrcLineData->m_strId, pDesLineData->m_strId);
			}
		}
	}

	// for-for
	{
		int size = static_cast<int>(vecLineData.size());
		for (int i = 0; i < size; ++i)
		{
			TestLineData* pSrcLineData = vecLineData[i];

			for (int j = 0; j < size; ++j)
			{
				TestLineData* pDesLineData = vecLineData[j];
				if (pSrcLineData == pDesLineData)
					continue;
				bool isLink = false;
				if (isSamePt(pSrcLineData->m_ptBegin, pDesLineData->m_ptBegin))
					isLink = true;
				else if (isSamePt(pSrcLineData->m_ptBegin, pDesLineData->m_ptEnd))
					isLink = true;
				else if (isSamePt(pSrcLineData->m_ptEnd, pDesLineData->m_ptBegin))
					isLink = true;
				else if (isSamePt(pSrcLineData->m_ptEnd, pDesLineData->m_ptEnd))
					isLink = true;

				if (!isLink)
					continue;

				_testAccuracy(mapLine2Links2, pSrcLineData->m_strId, pDesLineData->m_strId);
			}
		}
	}

	// 保存
	{
		Sindy::SQLite database(strDbPath);
		database.beginTransaction();
		database.dropTable("result_range");
		database.execute("CREATE TABLE result_range(handle TEXT PRIMARY KEY,relation TEXT)");

		database.prepare("INSERT INTO result_range(handle,relation) VALUES(:1,:2)");
		for (const auto& item : mapLine2Links1)
		{
			std::string str;
			Sindy::joinStr(*(item.second), str);

			database.bindText(":1", item.first);
			database.bindText(":2", str);
			database.step();
			database.resetSyntax();
		}
		database.commit();
	}

	// for-for
	{
		int count = 0;
		Sindy::SQLite database(strDbPath);
		database.beginTransaction();
		database.dropTable("result_for");
		database.execute("CREATE TABLE result_for(handle TEXT PRIMARY KEY,relation TEXT)");

		database.prepare("INSERT INTO result_for(handle,relation) VALUES(:1,:2)");
		for (const auto& item : mapLine2Links2)
		{
			std::string strFor;
			Sindy::joinStr(*(item.second), strFor);

			database.bindText(":1", item.first);
			database.bindText(":2", strFor);
			database.step();
			database.resetSyntax();

			if (true)
			{
				auto iter = mapLine2Links1.find(item.first);
				if (iter == mapLine2Links1.end())
				{
					++count;
					std::cout << "key[" << item.first << "]没有find到" << std::endl;
					continue;
				}

				std::string strRange;
				Sindy::joinStr(*(iter->second), strRange);
				if (strFor != strRange)
				{
					++count;
					std::cout << "key[" << item.first << "]相交的外包不匹配：" << strFor << "和" << strRange << std::endl;
					continue;
				}
			}
		}
		database.commit();
		std::cout << count << "条记录不匹配" << std::endl;
	}
	Sindy::ReleaseMapValue(mapLine2Links1);
	Sindy::ReleaseMapValue(mapLine2Links2);
}

void TestPerformance::_testAccuracy(std::map<std::string, std::set<std::string>*>& mapLine2Links, const std::string& strLine1, const std::string& strLine2)
{
	if (strLine1 == strLine2)
		return;

	std::set<std::string>* pRelation = _getRelations(mapLine2Links, strLine1);
	pRelation->insert(strLine2);

	pRelation = _getRelations(mapLine2Links, strLine2);
	pRelation->insert(strLine1);
}

std::set<std::string>* TestPerformance::_getRelations(std::map<std::string, std::set<std::string>*>& mapLine2Links, const std::string& strId)
{
	auto iter = mapLine2Links.find(strId);
	if (iter != mapLine2Links.end())
		return iter->second;

	std::set<std::string>* pNew = new std::set<std::string>;
	mapLine2Links[strId] = pNew;
	return pNew;
}

void TestPerformance::testTraverse()
{
	std::vector<int> arr = { 0,1,2,3,4 };

	auto size = arr.size();
	for (auto i = 0; i < size; ++i)
	{
		for (auto j = i + 1; j < size; ++j)
		{
			std::cout << "[i,j]" << i << "," << j << std::endl;
		}
	}
}
