#pragma once
#include <string>
#include <map>

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

} // namespace Sindy