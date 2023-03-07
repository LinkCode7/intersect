#pragma once
#include <string>
#include <map>

#include "../SindyGlobal.h"
#include "../SQLite/SindySQLite.h"

#ifdef _WIN64
typedef __int64             SindyLong;
typedef unsigned __int64    SindyULong;
typedef __int64             SindyInt;
typedef unsigned __int64    SindyUInt;
#else
// Using __w64 let's us catch potential errors at compile time
// when /Wp64 is enabled.  Also, we use long, instead of int,
// in the 32-bit build.  That's for compatibility with the Int32
// and UInt32 types.
//
typedef __w64 long          SindyLong;
typedef __w64 unsigned long SindyULong;
//
typedef __w64 int           SindyInt;
typedef __w64 unsigned int  SindyUInt;
#endif

#if PMVCCUR_VER < PMVC2005_VER
typedef long REGIONID;
#else
typedef SindyInt REGIONID;
#endif

#define SINDY_API
#define SINDY_DEFAULT_DOUBLE 0.0

namespace Sindy
{
	class Point3d
	{
	public:
		Point3d()
		{
			x = SINDY_DEFAULT_DOUBLE;
			y = SINDY_DEFAULT_DOUBLE;
			z = SINDY_DEFAULT_DOUBLE;
		}
		Point3d(const Point3d& pt)
		{
			x = pt.x;
			y = pt.y;
			z = pt.z;
		}
		Point3d(double dX, double dY, double dZ)
		{
			x = dX;
			y = dY;
			z = dZ;
		}

		double x, y, z;
	};

	class Extents
	{
	public:
		Point3d min() const { return m_min; }
		Point3d max() const { return m_max; }

		void reset(const Point3d& min, const Point3d& max) { m_min = min; m_max = max; }
		bool outExtents(const Extents& ext, double tol = 0.0)
		{
			if (ext.m_max.x < m_min.x - tol || ext.m_min.x > m_max.x + tol)
				return true;
			if (ext.m_max.y < m_min.y - tol || ext.m_min.y > m_max.y + tol)
				return true;
			return false;
		}

	private:
		Point3d m_min;
		Point3d m_max;
	};


	inline double getAbs(double dValue)
	{
		if (dValue > 0)
			return dValue;
		else
			return -dValue;
	}

	// 比较两个浮点数：0表示相同 1表示前面大 -1表示后面大
	SINDY_API int compareDbl(double src1, double src2, double dTol = 0.000001);

	inline bool isSamePt(const Sindy::Point3d& ptSrc, const Sindy::Point3d& ptDes, double dTol = 1.0)
	{
		if (fabs(ptSrc.x - ptDes.x) <= dTol && fabs(ptSrc.y - ptDes.y) <= dTol)
			return true;
		return false;
	}

	template<typename Container>
	void joinStr(const Container& container, std::string& str, const std::string& split = ",")
	{
		if (container.empty())
			return;

		for (const auto& item : container)
		{
			str += item;
			str += split;
		}
		str.erase(str.end() - 1);
	}

	template<typename Key, typename Value>
	inline void ReleaseMapValue(std::map<Key, Value*>& map)
	{
		for (auto& item : map)
			delete (item.second);
		map.clear();
	}

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

	class TestLineData
	{
	public:
		std::string m_strId;
		Sindy::Point3d m_ptBegin;
		Sindy::Point3d m_ptEnd;
		double m_dBulge = 0.0;

		Sindy::Extents m_extents;
	};

	// 反序列化测试数据
	template<typename LineDataType>
	void unSerializePoints(const PString& strDbPath, std::vector<LineDataType*>& vecLineData)
	{
		// TestData.Line
		Sindy::SQLite database(strDbPath);

		std::string strSql = "select * from ";
		strSql += Sindy::table_name;

		database.beginTransaction();
		database.prepare(strSql);

		while (SQLITE_ROW == database.step())
		{
			LineDataType* pLineData = new LineDataType();
			database.getValueText(Sindy::handle, pLineData->m_strId);

			database.getValueDouble(Sindy::bulge, pLineData->m_dBulge);
			database.getValueDouble(Sindy::from_x, pLineData->m_ptBegin.x);
			database.getValueDouble(Sindy::from_y, pLineData->m_ptBegin.y);
			database.getValueDouble(Sindy::to_x, pLineData->m_ptEnd.x);
			database.getValueDouble(Sindy::to_y, pLineData->m_ptEnd.y);

			double dMinX = 0.0;
			database.getValueDouble(Sindy::min_x, dMinX);
			double dMinY = 0.0;
			database.getValueDouble(Sindy::min_y, dMinY);
			double dMaxX = 0.0;
			database.getValueDouble(Sindy::max_x, dMaxX);
			double dMaxY = 0.0;
			database.getValueDouble(Sindy::max_y, dMaxY);

			pLineData->m_extents.reset(Sindy::Point3d(dMinX, dMinY, 0.0), Sindy::Point3d(dMaxX, dMaxY, 0.0));

			vecLineData.push_back(pLineData);
		}
	}

} // namespace Sindy