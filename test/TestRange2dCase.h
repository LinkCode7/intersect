#ifndef TEST_RANGE2D_CASE_H
#define TEST_RANGE2D_CASE_H

#include "../common/SindyUtility.h"
#include "../bound/RangeBound.h"

namespace TestRange2dCase
{
	class LineData2 : public Sindy::TestLineData, public Sindy::IBoundItem
	{
	public:
		LineData2(const Sindy::Point3d& ptMin, const Sindy::Point3d& ptMax, const std::string& handle)
		{
			m_ptBegin = ptMin;
			m_ptEnd = ptMax;
			m_strId = handle;
		}
		bool getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY) override
		{
			dMinX = m_ptBegin.x;
			dMinY = m_ptBegin.y;
			dMaxX = m_ptEnd.x;
			dMaxY = m_ptEnd.y;
			return true;
		}
		bool getId(REGIONID& id) override
		{
			id = std::atoi(m_strId.data());
			return true;
		}
	};

	class BoxInfo
	{
	public:
		Sindy::Point3d m_ptMin;
		Sindy::Point3d m_ptMax;
		std::string m_strHandle;
		BoxInfo(const Sindy::Point3d& ptMin, const Sindy::Point3d& ptMax, const std::string& strHandle) : m_ptMin(ptMin), m_ptMax(ptMax), m_strHandle(strHandle) {}
	};

	void entry();
	void testIntersect();
	void testBoundSort();
	void testSrcDes();

	std::vector<std::shared_ptr<LineData2>> _makeLineData(const std::vector<BoxInfo>& arrBox);
	int check(const std::vector<BoxInfo>& arrBox, const std::vector<std::string>& expect, double tol = 0.0);
}

#endif // !TEST_RANGE2D_CASE_H
