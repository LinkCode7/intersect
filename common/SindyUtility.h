#pragma once
#include <map>
#include <string>

#include "../SindyGlobal.h"
#include "../sqlite/SindySQLite.h"
#include "SindyExtents.h"

#ifdef _WIN64
typedef __int64          SindyLong;
typedef unsigned __int64 SindyULong;
typedef __int64          SindyInt;
typedef unsigned __int64 SindyUInt;
#else
// Using __w64 let's us catch potential errors at compile time
// when /Wp64 is enabled.  Also, we use long, instead of int,
// in the 32-bit build.  That's for compatibility with the Int32
// and UInt32 types.
//
typedef __w64 long          SindyLong;
typedef __w64 unsigned long SindyULong;
//
typedef __w64 int          SindyInt;
typedef __w64 unsigned int SindyUInt;
#endif

#if PMVCCUR_VER < PMVC2005_VER
typedef long REGIONID;
#else
typedef SindyInt           REGIONID;
#endif

#define SINDY_API

namespace sindy
{
inline double getAbs(double dValue)
{
    if (dValue > 0)
        return dValue;
    else
        return -dValue;
}

// 比较两个浮点数：0表示相同 1表示前面大 -1表示后面大
SINDY_API int compareDbl(double src1, double src2, double dTol = 0.000001);

inline bool isSamePt(const sindy::Point3d& ptSrc, const sindy::Point3d& ptDes, double dTol = 1.0)
{
    if (fabs(ptSrc.x - ptDes.x) <= dTol && fabs(ptSrc.y - ptDes.y) <= dTol)
        return true;
    return false;
}

template <typename Container>
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

template <typename Key, typename Value>
inline void ReleaseMapValue(std::map<Key, Value*>& map)
{
    for (auto& item : map)
        delete (item.second);
    map.clear();
}

const char table_name[] = "Line";

const char handle[] = "handle";
const char bulge[]  = "bulge";

const char from_x[] = "from_x";
const char from_y[] = "from_y";
const char to_x[]   = "to_x";
const char to_y[]   = "to_y";

const char min_x[] = "min_x";
const char min_y[] = "min_y";
const char max_x[] = "max_x";
const char max_y[] = "max_y";

class TestLineData
{
public:
    std::string    m_strId;
    sindy::Point3d m_ptBegin;
    sindy::Point3d m_ptEnd;
    double         m_dBulge = 0.0;

    sindy::Extents m_extents;
};

// 反序列化测试数据
template <typename LineDataType>
void unSerializePoints(const PString& strDbPath, std::vector<LineDataType*>& vecLineData)
{
    // TestData.Line
    sindy::SQLite database(strDbPath);

    std::string strSql = "select * from ";
    strSql += sindy::table_name;

    database.beginTransaction();
    database.prepare(strSql);

    while (SQLITE_ROW == database.step())
    {
        LineDataType* pLineData = new LineDataType();
        database.getValueText(sindy::handle, pLineData->m_strId);

        database.getValueDouble(sindy::bulge, pLineData->m_dBulge);
        database.getValueDouble(sindy::from_x, pLineData->m_ptBegin.x);
        database.getValueDouble(sindy::from_y, pLineData->m_ptBegin.y);
        database.getValueDouble(sindy::to_x, pLineData->m_ptEnd.x);
        database.getValueDouble(sindy::to_y, pLineData->m_ptEnd.y);

        double dMinX = 0.0;
        database.getValueDouble(sindy::min_x, dMinX);
        double dMinY = 0.0;
        database.getValueDouble(sindy::min_y, dMinY);
        double dMaxX = 0.0;
        database.getValueDouble(sindy::max_x, dMaxX);
        double dMaxY = 0.0;
        database.getValueDouble(sindy::max_y, dMaxY);

        pLineData->m_extents = {sindy::Point3d(dMinX, dMinY, 0.0), sindy::Point3d(dMaxX, dMaxY, 0.0)};

        vecLineData.push_back(pLineData);
    }
}

} // namespace sindy