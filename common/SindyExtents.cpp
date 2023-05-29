#include "SindyExtents.h"

#include "float.h"

#define MY_EXTENTS_MAX -DBL_MAX
#define MY_EXTENTS_MIN DBL_MAX

bool compareDouble(double value1, double value2, double tol = SINDY_ZERO)
{
    double sub = value1 - value2;
    if (sub < 0)
        sub = -sub;

    if (sub <= tol)
        return true;
    return false;
}

namespace sindy
{
Extents::Extents() : m_min{MY_EXTENTS_MIN, MY_EXTENTS_MIN, MY_EXTENTS_MIN}, m_max{MY_EXTENTS_MAX, MY_EXTENTS_MAX, MY_EXTENTS_MAX}
{
}
Extents::Extents(const Point3d& pt) : m_min(pt), m_max(pt)
{
}
Extents::Extents(const std::initializer_list<Point3d>& list)
    : m_min{MY_EXTENTS_MIN, MY_EXTENTS_MIN, MY_EXTENTS_MIN}, m_max{MY_EXTENTS_MAX, MY_EXTENTS_MAX, MY_EXTENTS_MAX}
{
    for (const auto& point : list)
        addPoint(point);
}

void Extents::reset()
{
    m_min = {MY_EXTENTS_MIN, MY_EXTENTS_MIN, MY_EXTENTS_MIN};
    m_max = {MY_EXTENTS_MAX, MY_EXTENTS_MAX, MY_EXTENTS_MAX};
}
void Extents::reset(const Point3d& pt1, const Point3d& pt2)
{
    m_min = pt1;
    m_max = pt1;
    addPoint(pt2);
}
void Extents::set(const Point3d& ptMin, const Point3d& ptMax)
{
    m_min = ptMin;
    m_max = ptMax;
}

// 建议使用前先判断有效性
bool Extents::invalid() const
{
    if (compareDouble(m_min.x, MY_EXTENTS_MIN) && compareDouble(m_min.y, MY_EXTENTS_MIN) && compareDouble(m_min.z, MY_EXTENTS_MIN) &&
        compareDouble(m_max.x, MY_EXTENTS_MAX) && compareDouble(m_max.y, MY_EXTENTS_MAX) && compareDouble(m_max.z, MY_EXTENTS_MAX))
        return true;
    return false;
}
void Extents::addPoint(const Point3d& pt)
{
    if (pt.x < m_min.x)
        m_min.x = pt.x;
    if (pt.x > m_max.x)
        m_max.x = pt.x;

    if (pt.y < m_min.y)
        m_min.y = pt.y;
    if (pt.y > m_max.y)
        m_max.y = pt.y;

    if (pt.z < m_min.z)
        m_min.z = pt.z;
    if (pt.z > m_max.z)
        m_max.z = pt.z;
}
void Extents::addExtents(const Extents& ext)
{
    addPoint(ext.m_min);
    addPoint(ext.m_max);
}
void Extents::operator+=(const Extents& ext)
{
    addPoint(ext.m_min);
    addPoint(ext.m_max);
}

bool Extents::inExtents(const Point3d& pt) const
{
    if (pt.x < m_min.x || pt.x > m_max.x)
        return false;
    if (pt.y < m_min.y || pt.y > m_max.y)
        return false;
    if (pt.z < m_min.z || pt.z > m_max.z)
        return false;
    return true;
}
bool Extents::inExtents(const Point3d& pt, double tol) const
{
    if (pt.x < m_min.x - tol || pt.x > m_max.x + tol)
        return false;
    if (pt.y < m_min.y - tol || pt.y > m_max.y + tol)
        return false;
    if (pt.z < m_min.z - tol || pt.z > m_max.z + tol)
        return false;
    return true;
}
bool Extents::outExtents(const Point3d& pt) const
{
    if (pt.x < m_min.x || pt.x > m_max.x)
        return true;
    if (pt.y < m_min.y || pt.y > m_max.y)
        return true;
    if (pt.z < m_min.z || pt.z > m_max.z)
        return true;
    return false;
}
bool Extents::outExtents(const Point3d& pt, double tol) const
{
    if (pt.x < m_min.x - tol || pt.x > m_max.x + tol)
        return true;
    if (pt.y < m_min.y - tol || pt.y > m_max.y + tol)
        return true;
    if (pt.z < m_min.z - tol || pt.z > m_max.z + tol)
        return true;
    return false;
}
bool Extents::outExtents(const Extents& ext) const
{
    if (ext.m_max.x < m_min.x || ext.m_min.x > m_max.x)
        return true;
    if (ext.m_max.y < m_min.y || ext.m_min.y > m_max.y)
        return true;
    if (ext.m_max.z < m_min.z || ext.m_min.z > m_max.z)
        return true;
    return false;
}
bool Extents::outExtents(const Extents& ext, double tol) const
{
    if (ext.m_max.x < m_min.x - tol || ext.m_min.x > m_max.x + tol)
        return true;
    if (ext.m_max.y < m_min.y - tol || ext.m_min.y > m_max.y + tol)
        return true;
    if (ext.m_max.z < m_min.z - tol || ext.m_min.z > m_max.z + tol)
        return true;
    return false;
}

void Extents::expand(double value)
{
    m_min.x -= value;
    m_min.y -= value;
    m_min.z -= value;
    m_max.x += value;
    m_max.y += value;
    m_max.z += value;
}

void Extents::moveTo(const Point3d& ptNewCenter)
{
    Point3d ptCurCenter{(m_max.x * 0.5 + m_min.x * 0.5), (m_max.y * 0.5 + m_min.y * 0.5), (m_max.z * 0.5 + m_min.z * 0.5)};
    double  offsetX = ptNewCenter.x - ptCurCenter.x;
    double  offsetY = ptNewCenter.y - ptCurCenter.y;
    double  offsetZ = ptNewCenter.z - ptCurCenter.z;
    m_min.x += offsetX;
    m_min.y += offsetY;
    m_min.z += offsetZ;
    m_max.x += offsetX;
    m_max.y += offsetY;
    m_max.z += offsetZ;
}

Point3d Extents::centerPt() const
{
    return {(m_max.x * 0.5 + m_min.x * 0.5), (m_max.y * 0.5 + m_min.y * 0.5), (m_max.z * 0.5 + m_min.z * 0.5)};
}

} // namespace sindy
