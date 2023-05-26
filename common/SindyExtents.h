#ifndef SINDY_EXTENTS_H
#define SINDY_EXTENTS_H
#include <initializer_list>

#define SINDY_ZERO 0.00001  // 1e-5
#define SINDY_DEFAULT_DOUBLE 0.0

namespace sindy
{
class Point3d
{
public:
    double x;
    double y;
    double z;
    Point3d(const Point3d& pt) : x(pt.x), y(pt.y), z(pt.z) {}
    Point3d(double dX, double dY, double dZ) : x(dX), y(dY), z(dZ) {}
    Point3d(double dX, double dY) : x(dX), y(dY), z(SINDY_DEFAULT_DOUBLE) {}
    Point3d() : x(SINDY_DEFAULT_DOUBLE), y(SINDY_DEFAULT_DOUBLE), z(SINDY_DEFAULT_DOUBLE) {}
};

// 用一个最小的平行于坐标轴的矩形来框住几何体：Minimum Bounding Box
class Extents
{
    Point3d m_min;
    Point3d m_max;

public:
    Extents();
    Extents(const Point3d& pt);
    explicit Extents(const std::initializer_list<Point3d>& list);
    void           set(const Point3d& ptMin, const Point3d& ptMax);
    inline Point3d min() const { return m_min; }
    inline Point3d max() const { return m_max; }

    void    reset();
    void    reset(const Point3d& pt1, const Point3d& pt2);
    bool    invalid() const;
    Point3d centerPt() const;
    void    expand(double value);  // 扩大或缩小(负数)包络
    void    moveTo(const Point3d& ptNewCenter);

    void addPoint(const Point3d& pt);
    void addExtents(const Extents& ext);
    void operator+=(const Extents& ext);
    bool inExtents(const Point3d& pt) const;
    bool inExtents(const Point3d& pt, double tol) const;
    bool outExtents(const Point3d& pt) const;
    bool outExtents(const Point3d& pt, double tol) const;
    bool outExtents(const Extents& ext) const;
    bool outExtents(const Extents& ext, double tol) const;
};

}  // namespace sindy

#endif  // !SINDY_EXTENTS_H