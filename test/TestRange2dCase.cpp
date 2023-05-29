#include "TestRange2dCase.h"

#include <cassert>
#include <memory>

void TestRange2dCase::entry()
{
    testBoundSort();
    testIntersect();

    randBox();
    testSrcDes();
}

std::vector<std::shared_ptr<TestRange2dCase::LineData2>> TestRange2dCase::_makeLineData(const std::vector<BoxInfo>& arrBox)
{
    std::vector<std::shared_ptr<LineData2>> arrLineData;
    for (const auto& box : arrBox)
    {
        arrLineData.push_back(std::make_shared<LineData2>(box.m_ptMin, box.m_ptMax, box.m_strHandle));
    }
    return arrLineData;
}

std::set<std::string> TestRange2dCase::_getIntersectResult(sindy::Range2d& range, const std::vector<std::shared_ptr<LineData2>>& lineDatas,
                                                           double tol)
{
    range.setRangeItems(lineDatas, true, tol);

    std::vector<sindy::RangeItem*> vecItem;
    range.getIntersectItem(vecItem);

    std::set<std::string> result;
    for (auto iter = vecItem.begin(); iter != vecItem.end(); ++iter)
    {
        sindy::RangeItem* pSrcItem     = *iter;
        auto              pSrcLineData = pSrcItem->cast<LineData2>();

        auto it = pSrcItem->begin();
        for (; it != pSrcItem->end(); ++it)
        {
            auto pDesLineData = (*it)->cast<LineData2>();

            std::string relation;
            if (std::atoi(pSrcLineData->m_strId.c_str()) < std::atoi(pDesLineData->m_strId.c_str()))
                relation = pSrcLineData->m_strId + "-" + pDesLineData->m_strId;
            else
                relation = pDesLineData->m_strId + "-" + pSrcLineData->m_strId;
            result.emplace(relation);
        }
    }
    return result;
}

int TestRange2dCase::check(const std::vector<BoxInfo>& arrBox, const std::vector<std::string>& expect, double tol)
{
    auto arrLineData = _makeLineData(arrBox);

    sindy::Range2d        range;
    std::set<std::string> result = _getIntersectResult(range, arrLineData, tol);

#ifdef _DEBUG
    range.reset();
    range.setRangeItems(arrLineData, true, tol);
    std::vector<double> resultSort = range.testSortBox();
#endif

    if (result.size() != expect.size())
        return 0;

    for (const auto& str : expect)
    {
        if (result.find(str) == result.end())
            return 0;
    }

    return 1;
}

void TestRange2dCase::testIntersect()
{
    std::vector<std::string> expect = {"1-2"};

    int flag = 1;

    // 边界恰好相交
    flag &= check({{{0, 0}, {1, 1}, "1"}, {{1, 1}, {2, 2}, "2"}}, expect);
    flag &= check({{{0, 0}, {1, 1}, "1"}, {{1, 1}, {2, 2}, "2"}}, expect, 0.001);

    flag &= check({{{10, 20}, {15, 30}, "1"}, {{3, 5}, {10, 20}, "2"}}, expect, 0.001);
    flag &= check({{{3, 5}, {10, 20}, "2"}, {{10, 20}, {15, 30}, "1"}}, expect);
    flag &= check({{{10, 20}, {15, 30}, "1"}, {{3, 5}, {10, 20}, "2"}}, expect);

    // 普通相交
    flag &= check({{{-10.5, -10.5}, {-3.1, -3.1}, "1"}, {{-10.9, -10.9}, {-7.3, 1.1}, "2"}}, expect);    // 左上
    flag &= check({{{0, 0}, {1, 1}, "1"}, {{0.99, 0.99}, {2, 2}, "2"}}, expect);                         // 右上
    flag &= check({{{-20, -20.5}, {5.5, 4.5}, "1"}, {{5.3, -20}, {10.8, -20}, "2"}}, expect);            // 右下
    flag &= check({{{-5.212, 5.9}, {-3.1567, 6.16}, "1"}, {{-9.63, -10.3}, {-5.21, 6.1}, "2"}}, expect); // 左下
    flag &= check({{{10, 20}, {15, 30}, "1"}, {{3, 5}, {10.001, 20.001}, "2"}}, expect);

    // 完全覆盖
    flag &= check({{{-10, -10}, {10, 10}, "1"}, {{-5.5, -5.5}, {5.5, 5.5}, "2"}}, expect);
    flag &= check({{{1, 1}, {10, 10}, "1"}, {{0.99, 0.99}, {9.9, 9.9}, "2"}}, expect, 0.01);

    // 完全相同
    flag &= check({{{0, 0}, {0, 0}, "1"}, {{0, 0}, {0, 0}, "2"}}, expect);
    flag &= check({{{-4.6, -4.6}, {-3.6, -4.6}, "1"}, {{-4.6, -4.6}, {-3.6, -4.6}, "2"}}, expect);

    // 不相交
    flag &= check({{{-10, -9}, {-3.3, -3.3}, "3"}, {{-3.3, -3.2999}, {3, 3}, "4"}}, {});       // box4.min.y > box3.max.y
    flag &= check({{{-10, -9}, {-3.3, -3.3}, "3"}, {{-100, -100}, {-3.3, -9.0001}, "4"}}, {}); // box4.max.y < box3.min.y
    flag &= check({{{0, 0}, {1, 1}, "3"}, {{1.001, 0}, {3, 3}, "4"}}, {});                     // box4.min.x > box3.max.x
    flag &= check({{{0, 0}, {1, 1}, "3"}, {{-9, -9}, {-0.001, 3}, "4"}}, {});                  // box4.max.x < box3.min.x

    flag &= check({{{-4.6, -4.6}, {-3.6, -4.6}, "3"}, {{-3.59, -4.6}, {-3.58, -4.6}, "4"}}, {}); // 直线，box4.min.x > box3.max.x

    flag &= check({{{1, 1}, {10, 10}, "1"}, {{0, 0}, {0.9, 0.9}, "2"}}, {"1-2"}, 0.1 * 0.5); // 0.1 * 0.5：两个box都扩大
    flag &= check({{{1, 1}, {10, 10}, "1"}, {{0, 0}, {0.9, 0.9}, "2"}}, {}, 0.1 * 0.5 - 0.0001);

    flag &= check({{{1, 1}, {1, 1}, "1"}, {{2, 2}, {2, 2}, "2"}}, {}); // 特殊值

    assert(flag == 1);
}

void TestRange2dCase::testBoundSort()
{
    using namespace sindy;

    auto checkSort = [](const std::vector<BoxInfo>& arrBox, const std::vector<double>& expect) -> int {
        Range2d range;
        auto    arrLineData = _makeLineData(arrBox);
        range.setRangeItems(arrLineData, true, 0.0);
        std::vector<double> result = range.testSortBox();

        auto size = result.size();
        if (size != expect.size())
            return 0;

        for (auto i = 0; i < size; ++i)
        {
            if (std::to_string(result[i]) != std::to_string(expect[i]))
                return 0;
        }
        return 1;
    };

    /*
     * 本测试用例只关注包围盒的X值，主要测试两个包围盒的最小点、最大点的X值相等时，包围盒item的顺序
     * 如：box1.min = {0,0};box1.max = {1,0};box2.min = {1,0};box2.max = {2,2};
     * 我们期望靠右的box排在前面，即：box2.min.x,box1.max.x
     * 返回值：整数部分为box的ID，小数部分只有一位，十分位0代表box最小点，1代表box最大点
     */
    int flag = 1;

    // 两两测试，注意期望值不是{ box1.min.x, box1.max.x, box2.min.x, box2.max.x }，即{ 1.0, 1.1, 2.0, 2.1 }
    std::vector<double> expect = {1.0, 2.0, 1.1, 2.1};

    flag &= checkSort({{{0, 0}, {1, 0}, "1"}, {{1, 0}, {2, 2}, "2"}}, expect);
    flag &= checkSort({{{1, 0}, {2, 2}, "2"}, {{0, 0}, {1, 0}, "1"}}, expect); // 调整顺序

    flag &= checkSort({{{0, 0}, {1, 1}, "1"}, {{1, 1}, {2, 2}, "2"}}, expect);
    flag &= checkSort({{{1, 1}, {2, 2}, "2"}, {{0, 0}, {1, 1}, "1"}}, expect); // 调整顺序

    flag &= checkSort({{{-1, -1}, {0, 0}, "1"}, {{0, 0}, {1, 1}, "2"}}, expect);
    flag &= checkSort({{{0, 0}, {1, 1}, "2"}, {{-1, -1}, {0, 0}, "1"}}, expect); // 调整顺序

    flag &= checkSort({{{0, 0}, {1, 1}, "1"}, {{0.99, 0.99}, {2, 2}, "2"}}, expect);

    flag &= checkSort({{{-5, -5}, {-3.14, -3.14}, "1"}, {{-3.14, 0}, {1, 1}, "2"}}, expect);
    flag &= checkSort({{{-3.14, 0}, {1, 1}, "2"}, {{-5, -5}, {-3.14, -3.14}, "1"}}, expect); // 调整顺序

    // 三个box
    std::vector<double> expect2 = {1.0, 2.0, 1.1, 3.0, 2.1, 3.1};
    flag &= checkSort({{{-99.5, -99.5}, {-80.5, 0}, "1"}, {{-80.5, 1.7}, {5.5, 5.5}, "2"}, {{5.5, 10.9}, {30.15, 40.9}, "3"}}, expect2);
    flag &= checkSort({{{5.5, 10.9}, {30.15, 40.9}, "3"}, {{-80.5, 1.7}, {5.5, 5.5}, "2"}, {{-99.5, -99.5}, {-80.5, 0}, "1"}},
                      expect2); // 调整顺序

    // 四个box
    std::vector<double>  expect3 = {1.0, 2.0, 1.1, 3.0, 2.1, 4.0, 3.1, 4.1};
    std::vector<BoxInfo> arrBox  = {{{
                                        1,
                                        1,
                                    },
                                    {
                                        2,
                                        2,
                                    },
                                    "1"},
                                   {{2, 2}, {3, 3}, "2"},
                                   {{3, 3}, {4, 4}, "3"},
                                   {{4, 4}, {5, 5}, "4"}};
    flag &= checkSort(arrBox, expect3);
    arrBox = {{{2, 2}, {3, 3}, "2"},
              {{4, 4}, {5, 5}, "4"},
              {{3, 3}, {4, 4}, "3"},
              {{
                   1,
                   1,
               },
               {
                   2,
                   2,
               },
               "1"}}; // 调整顺序
    flag &= checkSort(arrBox, expect3);

    // 特殊值
    flag &= checkSort({{{0, 0}, {0, 0}, "1"}, {{0, 0}, {2, 2}, "2"}}, expect);
    flag &= checkSort({{{0.618, 0.618}, {0.618, 0.618}, "1"}, {{0.618, 0.618}, {1.618, 1.618}, "2"}}, expect);
    flag &= checkSort({{{30.9, 30.9}, {30.9, 30.9}, "1"}, {{30.9, 30.9}, {30.9, 30.9}, "2"}}, expect);

    assert(flag == 1);
}

void TestRange2dCase::randBox()
{
#define MAX_BOX_COUNT 100
    std::vector<std::shared_ptr<LineData2>> arrLineData;

    sindy::Extents ext;
    for (auto i = 0; i < MAX_BOX_COUNT; ++i)
    {
        ext.reset({getFloatRand(), getFloatRand()}, {getFloatRand(), getFloatRand()});
        auto pLineData = std::make_shared<LineData2>(ext.min(), ext.max(), std::to_string(i));
        arrLineData.emplace_back(pLineData);
    }
#undef MAX_BOX_COUNT

    sindy::Range2d        range;
    std::set<std::string> result = _getIntersectResult(range, arrLineData, 0.0);

    auto count = 0;
    auto size  = arrLineData.size();
    for (int i = 0; i < size; ++i)
    {
        auto pSrcLineData = arrLineData[i];
        for (int j = i + 1; j < size; ++j)
        {
            auto pDesLineData = arrLineData[j];
            if (pSrcLineData->m_extents.outExtents(pDesLineData->m_extents))
                continue;

            std::string relation;
            if (std::atoi(pSrcLineData->m_strId.c_str()) < std::atoi(pDesLineData->m_strId.c_str()))
                relation = pSrcLineData->m_strId + "-" + pDesLineData->m_strId;
            else
                relation = pDesLineData->m_strId + "-" + pSrcLineData->m_strId;

            ++count;
            if (result.find(relation) == result.end())
            {
                assert(0);
                return;
            }
        }
    }

    if (result.size() != count)
    {
        assert(0);
        return;
    }
}

void TestRange2dCase::testSrcDes()
{
    /*
     * 特定场景下的使用方法：
     * 假定一张地图下有10万栋楼、1000个便利店，要想知道哪些楼下开了便利店（建立便利店和楼的关系）
     * 此时将srcBox设为数量较少的便利店，desBox设为数量较大的楼，来求两类实体的关联，速度会更快。
     */
    std::vector<BoxInfo> srcBox;
    auto                 srcLineData = _makeLineData(srcBox);

    std::vector<BoxInfo> desBox;
    auto                 desLineData = _makeLineData(desBox);

    sindy::Range2d range;
    range.setRangeItems(srcLineData, true, 10.0); // 设为源实体
    range.setRangeItems(desLineData, false);

    std::vector<sindy::RangeItem*> vecItem;
    range.getIntersectItem(vecItem);

    auto iter = vecItem.begin();
    for (; iter != vecItem.end(); ++iter)
    {
        sindy::RangeItem* pSrcItem     = *iter;
        auto              pSrcLineData = pSrcItem->cast<LineData2>();

        auto it = pSrcItem->begin();
        for (; it != pSrcItem->end(); ++it)
        {
            sindy::RangeItem* pDesItem     = *it;
            auto              pDesLineData = pDesItem->cast<LineData2>();

            // src-des intersect
        }
    }
}

float TestRange2dCase::getFloatRand()
{
    return rand() / ((double)(RAND_MAX) / 100);
}
