#include "TestBoostRtree.h"

#include "../Common/SindyUtility.h"
#include "../Common/SindyRunTime.h"

// to store queries results
#include <vector>
// just for output
#include <iostream>

#ifdef TEST_BOOST_RTREE
#include <sstream>
#include <codecvt>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>

#include <boost/geometry/index/rtree.hpp>

#include <boost/foreach.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
#endif

void TestBoostRtree::entry(const PString& strDbPath, const PString& strLogPath)
{
	using namespace Sindy;
	std::vector<TestLineData*> arrLineData;
	unSerializePoints(strDbPath, arrLineData);

	//queryOfpack(arrLineData, strDbPath, strLogPath);

	quickStart(arrLineData, strDbPath, strLogPath);
}

void TestBoostRtree::queryOfpack(const std::vector<Sindy::TestLineData*>& arrLineData, const PString& strDbPath, const PString& strLogPath)
{
#ifdef TEST_BOOST_RTREE
	using namespace Sindy;

	typedef bg::model::point<int, 2, bg::cs::cartesian> point;
	typedef bg::model::box<point> box;
	typedef std::pair<box, unsigned> value;

	RunTime time;

	size_t id_gen = 0;
	std::vector<value> cloud;
	for (const auto& pData : arrLineData)
	{
		box b(point(pData->m_extents.min().x, pData->m_extents.min().y), point(pData->m_extents.max().x, pData->m_extents.max().y));
		cloud.emplace_back(std::make_pair(b, id_gen++));
	}

	bgi::rtree< value, bgi::quadratic<16> > rtree(cloud);

	//time.addTimePoint("construct rtree-pack");

	for (const auto& bounding : cloud)
	{
		std::vector<value> result_s;
		rtree.query(bgi::intersects(bounding.first), std::back_inserter(result_s));
	}

	time.addTimePoint("");

	std::ostringstream oss;
	oss << "对" << arrLineData.size() << "个实体求交 -> 算法4：boost.geometry.index.rtree-pack";
	std::string strText = oss.str();

	time.write(strLogPath, strText);
#endif
}

void TestBoostRtree::quickStart(const std::vector<Sindy::TestLineData*>& arrLineData, const PString& strDbPath, const PString& strLogPath)
{
#ifdef TEST_BOOST_RTREE
	using namespace Sindy;

	typedef bg::model::point<int, 2, bg::cs::cartesian> point;
	typedef bg::model::box<point> box;
	typedef std::pair<box, unsigned> value;

	RunTime time;

	// create the rtree using default constructor
	bgi::rtree< value, bgi::quadratic<16> > rtree;

	// create some values
	unsigned i = 0;
	for (const auto& pData : arrLineData)
	{
		// create a box
		box b(point(pData->m_extents.min().x, pData->m_extents.min().y), point(pData->m_extents.max().x, pData->m_extents.max().y));
		// insert new value
		rtree.insert(std::make_pair(b, i++));
	}

	//time.addTimePoint("construct rtree");

	for (const auto& pData : arrLineData)
	{
		box b(point(pData->m_extents.min().x, pData->m_extents.min().y), point(pData->m_extents.max().x, pData->m_extents.max().y));

		std::vector<value> result_s;
		rtree.query(bgi::intersects(b), std::back_inserter(result_s));
	}

	time.addTimePoint("");

	std::ostringstream oss;
	oss << "对" << arrLineData.size() << "个实体求交 -> 算法4：boost.geometry.index.rtree";
	std::string strText = oss.str();

	time.write(strLogPath, strText);
#endif
}
