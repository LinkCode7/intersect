#pragma once



#define DEFAULT_DOUBLE 0.0

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





class Point3d
{
public:
	Point3d()
	{
		x = DEFAULT_DOUBLE;
		y = DEFAULT_DOUBLE;
		z = DEFAULT_DOUBLE;
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
	Point3d       minPoint() const { return m_min; }
	Point3d       maxPoint() const { return m_max; }

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
	Point3d    m_min;
	Point3d    m_max;
};
