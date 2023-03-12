## 简介
该仓库包含了一个性能较好的求交算法，用于计算大批量的几何图形之间的相交情况。它的优势在于一次性计算大量图形两两之间的相交情况，相同测试数据下比[boost.geometry.spatial_indexes.rtree](https://www.boost.org/doc/libs/1_81_0/libs/geometry/doc/html/geometry/spatial_indexes/)快很多。

## 组成
1. 算法核心在intersect/bound/RangeBound.cpp中。
2. 测试代码在intersect/test中，包含准确性测试、性能测试，测试结果会打印在TestResult.log中，如：
> 对84834个实体求交 -> 算法1：暴力遍历 总计：53910ms<br>
> 对84834个实体求交 -> 算法2：外包排斥 总计：12973ms<br>
> 对84834个实体求交 -> 算法3：Range2d 总计：460ms<br>
> 对84834个实体求交 -> 算法4：boost.geometry.index.rtree 总计：2247ms

3. 为了支持测试，项目中把84834个图形序列化到数据库TestData.db3中，intersect/sqlite/SindySQLite.cpp提供相应的读写函数。
4. intersect/common包含了一些计时等工具函数。

## 算法思路
1. 在计算几何中，每个图形都有一个外接矩形：Bounding，图形是否相交可以先判断Bounding是否相交。
2. Bounding相交可以用排斥的思路快速求出，你可以很容易实现下面的Extents类，注意inExtents、outExtents函数，最佳情况下只需要一次浮点数比较就能判断两个图形是否相交，这里的“快速排斥思想”是判断两个图形是否相交的基础。
```cpp
// SindyExtents.cpp
bool Extents::outExtents(const Point3d& pt, double tol) const
{
    if (pt.x < m_min.x - tol || pt.x > m_max.x + tol)
        return true;
    if (pt.y < m_min.y - tol || pt.y > m_max.y + tol)
        return true;
    return false;
}
```
3. 把计算几何中快速排斥的思想和排序算法相结合，就可以更快速地查询图形的相交情况，这是本算法的核心思想。
4. 用户通过Sindy::Range2d启动算法，把每个Bounding的最小点、最大点的X值分别创建item放进std::multimap中，算法启动时，会先创建Y方向的临时容器std::multimap，而后从小到大遍历X轴的容器。
5. 当遍历到某个item的最小点时，与此图形相交的其它图形开始创建两两关联，请注意mapY2Item.lower_bound(pSrcItem->m_dMinY)，此时不会找到min.x大于item.min.x的目标图形，但是后续遍历到目标图形时仍然会正确地创建两者的关联。
6. 当遍历到item的最大点时，与此图形相交的其它图形都找完了，所以把所有Y值为item.max.y的图形从mapY2Item中移除。
7. 请注意，这里是关键点：遍历到最小点时，会将item的最大点的Y值放进临时容器（请思考为什么是max.y）；遍历到最大点时，会将当前的max.y从临时容器移除。
8. 核心实现都在Range2d::getIntersectItem（创建节点数据时请使用配套的setItem函数），其它函数都是各种变体。

## 使用
1. 为了支持业务扩展，使用者需要把每个待计算的图形或图形数据类继承自IBoundItem，并至少重写getExtents函数，该函数用于获取图形的Bounding，这是必须的。getId可以返回某种标记，可以是数组索引、图形ID等，当你需要一个额外的值时。
2. 具体用法请参考测试代码。

## 构建
1. 本项目采用[CMake](https://cmake.org/)构建，CMake支持跨平台构建，几乎所有主流IDE都支持它，且不受IDE本身的版本影响。
2. 为了更高效地完成编码，项目内的使用了部分C++11的语法（特别是在测试用例中），如果您目前还在使用C++98标准，需要根据编译器提示做一些简单的等价替换。
3. 为了对比测试性能，项目中使用了boost.geometry库，但并未开启该部分测试代码。如要开启，请在CMakeLists中提供编译好的头文件、库路径。