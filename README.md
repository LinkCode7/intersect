## 简介
该仓库包含了一个性能较好的求交算法，用于计算大批量的图形之间的相交情况。

## 组成
1. 算法核心在intersect/RangeBound/RangeBound.cpp中。
2. 测试代码在intersect/Testing中，包含准确性测试、性能测试，测试结果会打印在TestResult.log中，如：
>对84834个实体求交 -> 算法1：暴力遍历 总计：121727ms<br>
>对84834个实体求交 -> 算法2：外包排斥 总计：35315ms<br>
>对84834个实体求交 -> 算法3：Range2d 总计：803ms

3. 为了支持测试，项目中把84834个图形序列化了到TestData.db3中，intersect/SQLite/SindySQLite.cpp提供相应的读写函数。
4. intersect/Common包含了一些计时等工具函数。

## 算法思路
1. 每个图形都有一个外接矩形：Bounding，图形是否相交可以先判断Bounding是否相交。
2. Bounding相交可以用排斥的思路快速求出，你可以很容易实现下面的Extents类，注意inExtents、outExtents函数，最佳情况下只需要一次浮点数比较就能判断两个图形是否相交。
```cpp
// .h
#define MY_ZERO 0.00001  // 1e-5

class MyPoint
{
public:
    double x;
    double y;
};

// 用一个最小的平行于坐标轴的矩形来框住几何体：Minimum Bounding Box
class MyExtents
{
    MyPoint m_min;
    MyPoint m_max;

public:
    MyExtents();
    MyExtents(const MyPoint &pt);
    MyExtents(const MyPoint &ptMin, const MyPoint &ptMax);
    void           set(const MyPoint &ptMin, const MyPoint &ptMax);
    inline MyPoint min() const { return m_min; }
    inline MyPoint max() const { return m_max; }

    void    reset();
    bool    invalid();
    MyPoint centerPt();
    void    expand(double value);  // 扩大或缩小(负数)包络
    void    moveTo(const MyPoint &ptNewCenter);

    void addPoint(const MyPoint &pt);
    void addExtents(const MyExtents &ext);
    void operator+=(const MyExtents &ext);
    bool inExtents(const MyPoint &pt, double tol = MY_ZERO) const;
    bool outExtents(const MyPoint &pt, double tol = MY_ZERO) const;
    bool outExtents(const MyExtents &ext, double tol = MY_ZERO) const;
};
```
```cpp
// .cpp
#include "float.h"

#define MY_EXTENTS_MAX -DBL_MAX
#define MY_EXTENTS_MIN DBL_MAX

bool compareDouble(double value1, double value2, double tol = MY_ZERO)
{
    double sub = value1 - value2;
    if (sub < 0)
        sub = -sub;

    if (sub <= tol)
        return true;
    return false;
}

MyExtents::MyExtents() : m_min{MY_EXTENTS_MIN, MY_EXTENTS_MIN}, m_max{MY_EXTENTS_MAX, MY_EXTENTS_MAX} {}
MyExtents::MyExtents(const MyPoint &pt) : m_min(pt), m_max(pt) {}
MyExtents::MyExtents(const MyPoint &ptMin, const MyPoint &ptMax) : m_min(ptMin), m_max(ptMax) {}

void MyExtents::reset()
{
    m_min = {MY_EXTENTS_MIN, MY_EXTENTS_MIN};
    m_max = {MY_EXTENTS_MAX, MY_EXTENTS_MAX};
}
void MyExtents::set(const MyPoint &ptMin, const MyPoint &ptMax)
{
    m_min = ptMin;
    m_max = ptMax;
}

bool MyExtents::invalid()
{
    if (compareDouble(m_min.x, MY_EXTENTS_MIN) && compareDouble(m_min.y, MY_EXTENTS_MIN) &&
        compareDouble(m_max.x, MY_EXTENTS_MAX) && compareDouble(m_max.y, MY_EXTENTS_MAX))
        return true;
    return false;
}
void MyExtents::addPoint(const MyPoint &pt)
{
    if (pt.x < m_min.x)
        m_min.x = pt.x;
    if (pt.x > m_max.x)
        m_max.x = pt.x;

    if (pt.y < m_min.y)
        m_min.y = pt.y;
    if (pt.y > m_max.y)
        m_max.y = pt.y;
}
void MyExtents::addExtents(const MyExtents &ext)
{
    addPoint(ext.m_min);
    addPoint(ext.m_max);
}
void MyExtents::operator+=(const MyExtents &ext)
{
    addPoint(ext.m_min);
    addPoint(ext.m_max);
}

bool MyExtents::inExtents(const MyPoint &pt, double tol) const
{
    if (pt.x < m_min.x - tol || pt.x > m_max.x + tol)
        return false;
    if (pt.y < m_min.y - tol || pt.y > m_max.y + tol)
        return false;
    return true;
}
bool MyExtents::outExtents(const MyPoint &pt, double tol) const
{
    if (pt.x < m_min.x - tol || pt.x > m_max.x + tol)
        return true;
    if (pt.y < m_min.y - tol || pt.y > m_max.y + tol)
        return true;
    return false;
}
bool MyExtents::outExtents(const MyExtents &ext, double tol) const
{
    if (ext.m_max.x < m_min.x - tol || ext.m_min.x > m_max.x + tol)
        return true;
    if (ext.m_max.y < m_min.y - tol || ext.m_min.y > m_max.y + tol)
        return true;
    return false;
}

void MyExtents::expand(double value)
{
    m_min.x -= value;
    m_min.y -= value;
    m_max.x += value;
    m_max.y += value;
}

void MyExtents::moveTo(const MyPoint &ptNewCenter)
{
    MyPoint ptCurCenter{(double(m_max.x * 0.5) + double(m_min.x * 0.5)), double((m_max.y * 0.5) + double(m_min.y * 0.5))};
    double   offsetX = ptNewCenter.x - ptCurCenter.x;
    double   offsetY = ptNewCenter.y - ptCurCenter.y;
    m_min.x += offsetX;
    m_min.y += offsetY;
    m_max.x += offsetX;
    m_max.y += offsetY;
}

MyPoint MyExtents::centerPt()
{
    MyPoint pt{(double(m_max.x * 0.5) + double(m_min.x * 0.5)), double((m_max.y * 0.5) + double(m_min.y * 0.5))};
    return pt;
}
```
3. 把几何图形学上的思想和排序算法结合，就可以更快速地查询图形的相交情况，这是算法核心思想。
4. 用户通过Sindy::Range2d启动算法，把每个Bounding的最小点、最大点的X值分别创建item放进std::multimap中，算法启动时，会先创建Y方向的临时容器std::multimap，而后从小到大遍历X轴的容器。
5. 当遍历到某个item的最小点时，与此图形相交的其它图形开始创建两两关联；当遍历到item的最大点时，与此图形相交的其它图形都找完了。在这之间，遍历到最小点时，会将item的最大点的Y值放进临时容器（请思考为什么是max.y）；遍历到最大点时，会将当前的max.y从临时容器移除。
6. 核心实现都在Range2d::GetIntersectItems，其它函数都是各种变体。

## 使用
1. 为了支持业务扩展，使用者需要把每个待计算的图形或图形数据类继承自IBoundItem，并至少重写GetExtents函数，该函数用于获取图形的Bounding，这是必须的。GetId可以返回某种标记，可以是数组索引、图形ID等，当你需要一个额外的值时。
2. 具体用法请参考测试代码。