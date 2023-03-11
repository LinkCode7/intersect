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
	m_dMinX(0.0),
	m_dMinY(0.0),
	m_dMaxX(0.0),
	m_dMaxY(0.0)
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
	if (!ipItem->getExtents(pMinItem->m_dMinX, pMinItem->m_dMinY, pMinItem->m_dMaxX, pMinItem->m_dMaxY))
	{
		delete pMinItem;
		delete pRange;
		return false;
	}

#ifdef _DEBUG
	if (pMinItem->m_dMaxX < pMinItem->m_dMinX || pMinItem->m_dMaxY < pMinItem->m_dMinY)
		return false; // error:invalid extents
#endif

	pMinItem->m_dMinX -= tol;
	pMinItem->m_dMinY -= tol;
	pMinItem->m_dMaxX += tol;
	pMinItem->m_dMaxY += tol;
	m_arrIndex.emplace_back(pMinItem);
	//m_mapDouble2Item.insert(std::make_pair(pMinItem->m_dMinX, pMinItem));

	RangeItem* pMaxItem = new RangeItem(ipItem, pRange, false, isSrc);
	pMaxItem->m_dMinX = pMinItem->m_dMinX;
	pMaxItem->m_dMinY = pMinItem->m_dMinY;
	pMaxItem->m_dMaxX = pMinItem->m_dMaxX;
	pMaxItem->m_dMaxY = pMinItem->m_dMaxY;
	m_arrIndex.emplace_back(pMaxItem);
	//m_mapDouble2Item.insert(std::make_pair(pMinItem->m_dMaxX, pMaxItem));

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
		else // 最小点
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
	}
}

void Sindy::Range2d::sortBox()
{
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