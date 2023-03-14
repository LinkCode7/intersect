#include "RangeBound.h"
#include <algorithm>

bool Sindy::IBoundItem::getId(REGIONID& id)
{
	return false;
}
bool Sindy::IBoundItem::getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY)
{
	return false;
}

Sindy::BoundItem::BoundItem(IBoundItem* ipItem) :
	m_ipItem(ipItem),
	m_minX(0.0),
	m_minY(0.0),
	m_maxX(0.0),
	m_maxY(0.0)
{
}

Sindy::RangeItem::RangeItem(IBoundItem* ipItem, Ranges* pRange, bool isMin, bool isSrc) :
	BoundItem(ipItem),
	m_pItems(pRange),
	m_maxValue(isMin ? 0 : 1),
	m_isSrc(isSrc)
{
}

Sindy::RangeItem::~RangeItem()
{
	if (m_maxValue) // 统一在某一端释放
		delete m_pItems;
}

Sindy::Range2d::~Range2d()
{
	reset();
}

void Sindy::Range2d::reset()
{
	for (auto& item : m_arrIndex)
		delete item;
	m_arrIndex.clear();
}

// 只输出源实体相关的Bound
bool Sindy::Range2d::setItem(IBoundItem* ipItem, bool isSrc, double tol)
{
	if (!ipItem) return false;

	RangeItem::Ranges* pRange = new(RangeItem::Ranges);
	RangeItem* pMinItem = new RangeItem(ipItem, pRange, true, isSrc);
	if (!ipItem->getExtents(pMinItem->m_minX, pMinItem->m_minY, pMinItem->m_maxX, pMinItem->m_maxY))
	{
		delete pMinItem;
		delete pRange;
		return false;
	}

#ifdef _DEBUG
	if (pMinItem->m_maxX < pMinItem->m_minX || pMinItem->m_maxY < pMinItem->m_minY)
		return false; // error:invalid extents
#endif

	pMinItem->m_minX -= tol;
	pMinItem->m_minY -= tol;
	pMinItem->m_maxX += tol;
	pMinItem->m_maxY += tol;
	m_arrIndex.emplace_back(pMinItem);
	//m_mapDouble2Item.insert(std::make_pair(pMinItem->m_minX, pMinItem));

	RangeItem* pMaxItem = new RangeItem(ipItem, pRange, false, isSrc);
	pMaxItem->m_minX = pMinItem->m_minX;
	pMaxItem->m_minY = pMinItem->m_minY;
	pMaxItem->m_maxX = pMinItem->m_maxX;
	pMaxItem->m_maxY = pMinItem->m_maxY;
	m_arrIndex.emplace_back(pMaxItem);
	//m_mapDouble2Item.insert(std::make_pair(pMinItem->m_maxX, pMaxItem));

	return true;
}

// setItem时要设置误差
void Sindy::Range2d::getIntersectItem(std::vector<RangeItem*>& vecIntersect, SrcDestFunction function)
{
	sortBox();

	std::multimap<double, BoundItem*, DoubleLess> mapY2Item;

	typedef std::multimap<double, BoundItem*, DoubleLess>::iterator MapIter;
	typedef std::pair<MapIter, MapIter> PairMapIter;

	for (const auto& pSrcItem : m_arrIndex)
	{
		if (pSrcItem->m_maxValue) // 最大点
		{
			// 取到当前Y
			PairMapIter pairIter = mapY2Item.equal_range(pSrcItem->m_maxY);

			while (pairIter.first != pairIter.second)
			{
				// 比较地址
				if (pairIter.first->second->m_ipItem == pSrcItem->m_ipItem)
				{
					// 添加相关Item
					if (pSrcItem->m_isSrc && !pSrcItem->m_pItems->m_items.empty())
						vecIntersect.push_back(pSrcItem);

					mapY2Item.erase(pairIter.first);
					break;
				}
				++pairIter.first;
			}
		}
		else // 最小点
		{
			// map.Max.y >= src.Min.y 为了支持完全覆盖的情况
			std::multimap<double, BoundItem*, DoubleLess>::iterator it = mapY2Item.lower_bound(pSrcItem->m_minY);

			for (; it != mapY2Item.end(); ++it)
			{
				RangeItem* pDestItem = static_cast<RangeItem*>(it->second);
				// 自定义传出数据
				if (!function(pSrcItem->m_isSrc, pDestItem->m_isSrc))
					continue;

				if (pDestItem->m_minY <= pSrcItem->m_maxY)
				{
					pSrcItem->m_pItems->m_items.push_back(pDestItem);
					pDestItem->m_pItems->m_items.push_back(pSrcItem);
				}
			}

			// 此容器的Key是MaxY
			mapY2Item.insert(std::make_pair(pSrcItem->m_maxY, pSrcItem));
		}
	}
}

void Sindy::Range2d::sortBox()
{
	// 从小到大排序，当值相等时，最大点的X对应的item 排在 最小点的X对应的item 之前，
	// 这是为了把仅在边界相交的两个box放到结果集中，而不依赖放进容器中的顺序，也是不用multimap的理由
	std::sort(m_arrIndex.begin(), m_arrIndex.end(), [](const auto& left, const auto& right) {
		if (left->value() == right->value() && left->m_maxValue != right->m_maxValue)
			return left->m_maxValue < right->m_maxValue;
		return left->value() < right->value();
	});
}

std::vector<double> Sindy::Range2d::testSortBox()
{
	sortBox();

	std::vector<double> result;
	for (const auto& pItem : m_arrIndex)
	{
		REGIONID id = 0;
		if (!pItem->m_ipItem->getId(id))
			continue; // error

		double value = id;
		if (pItem->m_maxValue)
			value += 0.1;
		result.emplace_back(value);
	}
	return result;
}

void Sindy::getSameItem(const std::vector<IBoundItem*>& items, const Point3d& ptMin, const Point3d& ptMax, std::set<IBoundItem*>& setRepeat, double tol)
{
	std::multimap<double, BoundItem*, DoubleLess> mapDouble2Item;

	for (const auto& ipItem : items)
	{
		if (!ipItem)
			continue;

		BoundItem* pItem = new BoundItem(ipItem);
		if (!ipItem->getExtents(pItem->m_minX, pItem->m_minY, pItem->m_maxX, pItem->m_maxY))
		{
			delete pItem;
			continue;
		}

		pItem->m_minX -= tol;
		pItem->m_minY -= tol;
		pItem->m_maxX += tol;
		pItem->m_maxY += tol;
		mapDouble2Item.insert(std::make_pair(pItem->m_minX, pItem)); // Min.X
	}

	double dLeft = ptMin.x - tol;
	double dRight = ptMax.x + tol;

	double dUp = ptMax.y + tol;
	double dDown = ptMin.y - tol;

	std::multimap<double, BoundItem*, DoubleLess>::const_iterator iter = mapDouble2Item.lower_bound(dLeft);

	for (; iter != mapDouble2Item.end(); ++iter)
	{
		if (iter->first > dRight)
			break;

		if (iter->second->m_maxX > dRight)
			continue;

		if (iter->second->m_maxY < dDown || iter->second->m_minY > dUp)
			continue;

		// 这里可能会进入两次
		setRepeat.insert(iter->second->boundItem());
	}
}

void Sindy::getSameItem(const std::vector<IBoundItem*>& items, const Point3d& ptInsert, std::set<IBoundItem*>& setRepeat, double radius, double tol)
{
	getSameItem(items, Point3d(ptInsert.x - radius, ptInsert.y - radius, 0), Point3d(ptInsert.x + radius, ptInsert.y + radius, 0), setRepeat, tol);
}

void Sindy::getIntersectItem2(const std::vector<IBoundItem*>& items, const Point3d& ptMin, const Point3d& ptMax, std::set<IBoundItem*>& setRepeat, double tol)
{
	std::multimap<double, BoundItem*, DoubleLess> mapDouble2Item;

	for (const auto& ipItem : items)
	{
		if (!ipItem)
			continue;

		BoundItem* pItem = new BoundItem(ipItem);
		if (!ipItem->getExtents(pItem->m_minX, pItem->m_minY, pItem->m_maxX, pItem->m_maxY))
		{
			delete pItem;
			continue;
		}

		pItem->m_minX -= tol;
		pItem->m_minY -= tol;
		pItem->m_maxX += tol;
		pItem->m_maxY += tol;
		mapDouble2Item.insert(std::make_pair(pItem->m_maxX, pItem)); // Max.X
	}


	double dLeft = ptMin.x - tol;
	double dRight = ptMax.x + tol;
	double dUp = ptMax.y + tol;
	double dDown = ptMin.y - tol;

	std::multimap<double, BoundItem*, DoubleLess>::const_iterator iter = mapDouble2Item.lower_bound(dLeft);
	for (; iter != mapDouble2Item.end(); ++iter)
	{
		if (iter->second->m_minX > dRight)
			continue;

		//if(iter->second->m_maxY >= dDown && iter->second->m_minY <= dUp)
		if (iter->second->m_minY > dUp || iter->second->m_maxY < dDown)
			continue;

		// 这里可能会进入两次
		setRepeat.insert(iter->second->boundItem());
	}
}