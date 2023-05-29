#ifndef TEST_RANGE2D_CASE_H
#define TEST_RANGE2D_CASE_H

#include "../bound/RangeBound.h"
#include "../common/SindyUtility.h"

namespace TestRange2dCase
{
// 入口
void entry();

// 各种相交、不相交情况测试
void testIntersect();
// 测试算法内部的重要排序规则，该顺序关系到边界相交时的结果稳定性
void testBoundSort();

// 通过随机地生成浮点数制造bounding，再和暴力遍历的结果对比，验证算法准确性
void randBox();
// 一种特定场景的推荐使用方法
void testSrcDes();

// 随机地生成浮点数
float getFloatRand();

class LineData2 : public sindy::TestLineData, public sindy::IBoundItem
{
public:
    LineData2(const sindy::Point3d& ptMin, const sindy::Point3d& ptMax, const std::string& handle)
    {
        m_ptBegin = ptMin;
        m_ptEnd   = ptMax;
        m_extents.reset(ptMin, ptMax);
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
    sindy::Point3d m_ptMin;
    sindy::Point3d m_ptMax;
    std::string    m_strHandle;
    BoxInfo(const sindy::Point3d& ptMin, const sindy::Point3d& ptMax, const std::string& strHandle)
        : m_ptMin(ptMin), m_ptMax(ptMax), m_strHandle(strHandle)
    {
    }
};

// 辅助函数
std::vector<std::shared_ptr<LineData2>> _makeLineData(const std::vector<BoxInfo>& arrBox);
std::set<std::string> _getIntersectResult(sindy::Range2d& range, const std::vector<std::shared_ptr<LineData2>>& lineDatas, double tol);
int                   check(const std::vector<BoxInfo>& arrBox, const std::vector<std::string>& expect, double tol = 0.0);
} // namespace TestRange2dCase

#endif // !TEST_RANGE2D_CASE_H
