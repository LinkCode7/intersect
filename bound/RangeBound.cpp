#include "RangeBound.h"

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
	m_dMinX(0.0),
	m_dMinY(0.0),
	m_dMaxX(0.0),
	m_dMaxY(0.0)
{
}

Sindy::RangeItem::RangeItem(IBoundItem* ipItem, Ranges* pRange, bool isMin, bool isSrc) :
	BoundItem(ipItem),
	m_pItems(pRange),
	m_isMin(isMin),
	m_isSrc(isSrc)
{
}

Sindy::RangeItem::~RangeItem()
{
	if (m_isMin) // 统一在最小端释放
		delete m_pItems;
}

Sindy::Range2d::~Range2d()
{
	reset();
}

void Sindy::Range2d::reset()
{
	std::multimap<double, BoundItem*, DoubleLess>::iterator iter = m_mapDouble2Item.begin();
	for (; iter != m_mapDouble2Item.end(); ++iter)
	{
		BoundItem* pItem = iter->second;
		delete pItem;
	}

	m_mapDouble2Item.clear();
}

// 只输出源实体相关的Bound
bool Sindy::Range2d::setItem(IBoundItem* ipItem, bool isSrc, double tol)
{
	if (!ipItem) return false;

	RangeItem::Ranges* pRange = new(RangeItem::Ranges);
	RangeItem* pMinItem = new RangeItem(ipItem, pRange, true, isSrc);
	if (!ipItem->getExtents(pMinItem->m_dMinX, pMinItem->m_dMinY, pMinItem->m_dMaxX, pMinItem->m_dMaxY))
	{
		delete pMinItem;
		delete pRange;
		return false;
	}

	pMinItem->m_dMinX -= tol;
	pMinItem->m_dMinY -= tol;
	pMinItem->m_dMaxX += tol;
	pMinItem->m_dMaxY += tol;

	m_mapDouble2Item.insert(std::make_pair(pMinItem->m_dMinX, pMinItem));


	RangeItem* pMaxItem = new RangeItem(ipItem, pRange, false, isSrc);
	pMaxItem->m_dMinX = pMinItem->m_dMinX;
	pMaxItem->m_dMinY = pMinItem->m_dMinY;
	pMaxItem->m_dMaxX = pMinItem->m_dMaxX;
	pMaxItem->m_dMaxY = pMinItem->m_dMaxY;

	m_mapDouble2Item.insert(std::make_pair(pMinItem->m_dMaxX, pMaxItem));

	return true;
}

// setItem时要设置误差
void Sindy::Range2d::getIntersectItem(std::vector<RangeItem*>& vecIntersect, SrcDestFunction function)
{
	std::multimap<double, BoundItem*, DoubleLess> mapY2Item;

	typedef std::multimap<double, BoundItem*, DoubleLess>::iterator MapIter;
	typedef std::pair<MapIter, MapIter> PairMapIter;

	std::multimap<double, BoundItem*, DoubleLess>::const_iterator iter = m_mapDouble2Item.begin();

	for (; iter != m_mapDouble2Item.end(); ++iter)
	{
		RangeItem* pSrcItem = static_cast<RangeItem*>(iter->second);

		if (pSrcItem->m_isMin) // 最小点
		{
			// map.Max.y >= src.Min.y 为了支持完全覆盖的情况
			std::multimap<double, BoundItem*, DoubleLess>::iterator it = mapY2Item.lower_bound(pSrcItem->m_dMinY);

			for (; it != mapY2Item.end(); ++it)
			{
				RangeItem* pDestItem = static_cast<RangeItem*>(it->second);
				// 自定义传出数据
				if (!function(pSrcItem->m_isSrc, pDestItem->m_isSrc))
					continue;

				if (pDestItem->m_dMinY <= pSrcItem->m_dMaxY)
				{
					pSrcItem->m_pItems->m_items.push_back(pDestItem);
					pDestItem->m_pItems->m_items.push_back(pSrcItem);
				}
			}

			// 此容器的Key是MaxY
			mapY2Item.insert(std::make_pair(pSrcItem->m_dMaxY, pSrcItem));
		}
		else // 最大点
		{
			// 取到当前Y
			PairMapIter pairIter = mapY2Item.equal_range(pSrcItem->m_dMaxY);

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
	}
}

void Sindy::getSameItem(const std::vector<IBoundItem*>& items, const Point3d& ptMin, const Point3d& ptMax, std::set<IBoundItem*>& setRepeat, double tol)
{
	std::multimap<double, BoundItem*, DoubleLess> mapDouble2Item;

	for (const auto& ipItem : items)
	{
		if (!ipItem)
			continue;

		BoundItem* pItem = new BoundItem(ipItem);
		if (!ipItem->getExtents(pItem->m_dMinX, pItem->m_dMinY, pItem->m_dMaxX, pItem->m_dMaxY))
		{
			delete pItem;
			continue;
		}

		pItem->m_dMinX -= tol;
		pItem->m_dMinY -= tol;
		pItem->m_dMaxX += tol;
		pItem->m_dMaxY += tol;
		mapDouble2Item.insert(std::make_pair(pItem->m_dMinX, pItem)); // Min.X
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

		if (iter->second->m_dMaxX > dRight)
			continue;

		if (iter->second->m_dMaxY < dDown || iter->second->m_dMinY > dUp)
			continue;

		// 这里可能会进入两次
		setRepeat.insert(iter->second->m_ipItem);
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
		if (!ipItem->getExtents(pItem->m_dMinX, pItem->m_dMinY, pItem->m_dMaxX, pItem->m_dMaxY))
		{
			delete pItem;
			continue;
		}

		pItem->m_dMinX -= tol;
		pItem->m_dMinY -= tol;
		pItem->m_dMaxX += tol;
		pItem->m_dMaxY += tol;
		mapDouble2Item.insert(std::make_pair(pItem->m_dMaxX, pItem)); // Max.X
	}


	double dLeft = ptMin.x - tol;
	double dRight = ptMax.x + tol;
	double dUp = ptMax.y + tol;
	double dDown = ptMin.y - tol;

	std::multimap<double, BoundItem*, DoubleLess>::const_iterator iter = mapDouble2Item.lower_bound(dLeft);
	for (; iter != mapDouble2Item.end(); ++iter)
	{
		if (iter->second->m_dMinX > dRight)
			continue;

		//if(iter->second->m_dMaxY >= dDown && iter->second->m_dMinY <= dUp)
		if (iter->second->m_dMinY > dUp || iter->second->m_dMaxY < dDown)
			continue;

		// 这里可能会进入两次
		setRepeat.insert(iter->second->m_ipItem);
	}
}