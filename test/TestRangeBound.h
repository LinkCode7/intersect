#pragma once
#include "../SindyGlobal.h"
#include "../bound/RangeBound.h"

namespace TestPerformance
{
void entry(const PString& strDbPath, const PString& strLogPath);

void testTraverse();
// 准确性
void testAccuracy(const PString& strDbPath);
void _testAccuracy(std::map<std::string, std::set<std::string>*>& mapLine2Links, const std::string& strLine1, const std::string& strLine2);
std::set<std::string>* _getRelations(std::map<std::string, std::set<std::string>*>& mapLine2Links, const std::string& strId);

class TestLineDataBound : public sindy::TestLineData, public sindy::IBoundItem
{
public:
    bool getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY) override
    {
        sindy::Point3d ptMin = m_extents.min();
        dMinX                = ptMin.x;
        dMinY                = ptMin.y;

        sindy::Point3d ptMax = m_extents.max();
        dMaxX                = ptMax.x;
        dMaxY                = ptMax.y;
        return true;
    }
};

void testRangeBound(const std::vector<TestLineDataBound*>& vecLineData, const PString& strLogPath);
void testFor(const std::vector<TestLineDataBound*>& vecLineData, const PString& strLogPath);
void testOutOf(const std::vector<TestLineDataBound*>& vecLineData, const PString& strLogPath);

}  // namespace TestPerformance