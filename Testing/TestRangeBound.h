#pragma once
#include "../SindyGlobal.h"
#include "../RangeBound/RangeBound.h"




namespace TestPerformance
{
	void entry(const PString& strDbPath, const PString& strLogPath);

	void testTraverse();
	// 准确性
	void testAccuracy(const PString& strDbPath);
	void _testAccuracy(std::map<std::string, std::set<std::string>*>& mapLine2Links, const std::string& strLine1, const std::string& strLine2);
	std::set<std::string>* _getRelations(std::map<std::string, std::set<std::string>*>& mapLine2Links, const std::string& strId);


	const char table_name[] = "Line";

	const char handle[] = "handle";
	const char bulge[] = "bulge";

	const char from_x[] = "from_x";
	const char from_y[] = "from_y";
	const char to_x[] = "to_x";
	const char to_y[] = "to_y";

	const char min_x[] = "min_x";
	const char min_y[] = "min_y";
	const char max_x[] = "max_x";
	const char max_y[] = "max_y";


	class TestLineData : public Sindy::IBoundItem
	{
	public:
		std::string m_strId;
		Point3d m_ptBegin;
		Point3d m_ptEnd;
		double m_dBulge = 0.0;

		Extents m_extents;

		bool GetExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY) override
		{
			Point3d ptMin = m_extents.minPoint();
			dMinX = ptMin.x;
			dMinY = ptMin.y;

			Point3d ptMax = m_extents.maxPoint();
			dMaxX = ptMax.x;
			dMaxY = ptMax.y;
			return true;
		}
	};


	inline bool isSamePt(const Point3d& ptSrc, const Point3d& ptDes, double dTol = 1.0)
	{
		if (fabs(ptSrc.x - ptDes.x) <= dTol && fabs(ptSrc.y - ptDes.y) <= dTol)
			return true;
		return false;
	}

	void unSerializePoints(const PString& strDbPath, std::vector<TestLineData*>& vecLineData);
	void testRangeBound(const std::vector<TestLineData*>& vecLineData, const PString& strLogPath);
	void testFor(const std::vector<TestLineData*>& vecLineData, const PString& strLogPath);
	void testOutOf(const std::vector<TestLineData*>& vecLineData, const PString& strLogPath);

} // TestPerformance