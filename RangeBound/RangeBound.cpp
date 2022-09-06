#include "RangeBound.h"



namespace Sindy
{
	bool IBoundItem::GetId(REGIONID& id)
	{
		return false;
	}
	bool IBoundItem::GetExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY)
	{
		return false;
	}

	BoundItem::BoundItem(IBoundItem* ipItem) :
		m_ipItem(ipItem),
		m_dMinX(0.0),
		m_dMinY(0.0),
		m_dMaxX(0.0),
		m_dMaxY(0.0)
	{
	}

	BoundItem::~BoundItem()
	{
	}

	RangeItem::RangeItem(IBoundItem* ipItem, Ranges* pRange, bool isMin, bool isSrc) :
		BoundItem(ipItem),
		m_pItems(pRange),
		m_isMin(isMin),
		m_isSrc(isSrc)
	{
	}

	RangeItem::~RangeItem()
	{
		if (m_isMin) // 统一在最小端释放
			delete(m_pItems);
	}

	Range2d::~Range2d()
	{
		Reset();
	}

	void Range2d::Reset()
	{
		std::multimap<double, BoundItem*, DoubleLess>::iterator iter = m_mapDouble2Item.begin();
		for (; iter != m_mapDouble2Item.end(); ++iter)
		{
			BoundItem* pItem = iter->second;
			delete(pItem);
		}

		m_mapDouble2Item.clear();

		m_setCheck.clear();
	}

	bool Range2d::SetItemMin(IBoundItem* ipItem, double dTol, bool isNeedCheckRepeat)
	{
		if (!ipItem) return false;

		// 请调用者保证Item唯一性
		if (isNeedCheckRepeat)
		{
			REGIONID id = UNKNOWN_REGION_ID;
			if (ipItem->GetId(id))
			{
				if (!m_setCheck.insert(id).second)
					return false;
			}
		}

		BoundItem* pItem = new BoundItem(ipItem);

		if (!ipItem->GetExtents(pItem->m_dMinX, pItem->m_dMinY, pItem->m_dMaxX, pItem->m_dMaxY))
		{
			delete(pItem);
			return false;
		}

		pItem->m_dMinX -= dTol;
		pItem->m_dMinY -= dTol;
		pItem->m_dMaxX += dTol;
		pItem->m_dMaxY += dTol;

		// Min.X
		m_mapDouble2Item.insert(std::make_pair(pItem->m_dMinX, pItem));

		return true;
	}

	void Range2d::GetSameItem(const Point3d& ptMin, const Point3d& ptMax, std::set<IBoundItem*>& setRepeat, double dTol) const
	{
		double dLeft = ptMin.x - dTol;
		double dRight = ptMax.x + dTol;

		double dUp = ptMax.y + dTol;
		double dDown = ptMin.y - dTol;

		std::multimap<double, BoundItem*, DoubleLess>::const_iterator iter = m_mapDouble2Item.lower_bound(dLeft);

		for (; iter != m_mapDouble2Item.end(); ++iter)
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

	void Range2d::GetSameItem(const Point3d& ptInsert, std::set<IBoundItem*>& setRepeat, double radius, double dTol) const
	{
		GetSameItem(Point3d(ptInsert.x - radius, ptInsert.y - radius, 0), Point3d(ptInsert.x + radius, ptInsert.y + radius, 0), setRepeat, dTol);
	}



	bool Range2d::SetItemMax(IBoundItem* ipItem, double dTol, bool isNeedCheckRepeat)
	{
		if (!ipItem) return false;

		// 请调用者保证Item唯一性
		if (isNeedCheckRepeat)
		{
			REGIONID id = UNKNOWN_REGION_ID;
			if (ipItem->GetId(id))
			{
				if (!m_setCheck.insert(id).second)
					return false;
			}
		}

		BoundItem* pItem = new BoundItem(ipItem);

		if (!ipItem->GetExtents(pItem->m_dMinX, pItem->m_dMinY, pItem->m_dMaxX, pItem->m_dMaxY))
		{
			delete(pItem);
			return false;
		}

		pItem->m_dMinX -= dTol;
		pItem->m_dMinY -= dTol;
		pItem->m_dMaxX += dTol;
		pItem->m_dMaxY += dTol;

		// Max.X
		m_mapDouble2Item.insert(std::make_pair(pItem->m_dMaxX, pItem));

		return true;
	}

	void Range2d::GetIntersectItem(const Point3d& ptMin, const Point3d& ptMax, std::set<IBoundItem*>& setRepeat, double dTol) const
	{
		double dLeft = ptMin.x - dTol;
		double dRight = ptMax.x + dTol;

		double dUp = ptMax.y + dTol;
		double dDown = ptMin.y - dTol;

		std::multimap<double, BoundItem*, DoubleLess>::const_iterator iter = m_mapDouble2Item.lower_bound(dLeft);

		for (; iter != m_mapDouble2Item.end(); ++iter)
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

	void Range2d::GetIntersectItem(const Point3d& ptInsert, std::set<IBoundItem*>& setRepeat, double radius, double dTol) const
	{
		GetIntersectItem(Point3d(ptInsert.x - radius, ptInsert.y - radius, 0), Point3d(ptInsert.x + radius, ptInsert.y + radius, 0), setRepeat, dTol);
	}






	// 只输出源实体相关的Bound
	bool Range2d::SetItems(IBoundItem* ipItem, bool isSrc, double dTol, bool isNeedCheckRepeat)
	{
		if (!ipItem) return false;

		// 请调用者保证Item唯一性
		if (isNeedCheckRepeat)
		{
			REGIONID id = UNKNOWN_REGION_ID;
			if (ipItem->GetId(id))
			{
				if (!m_setCheck.insert(id).second)
					return false;
			}
		}

		RangeItem::Ranges* pRange = new(RangeItem::Ranges);

		RangeItem* pMinItem = new RangeItem(ipItem, pRange, true, isSrc);
		if (!ipItem->GetExtents(pMinItem->m_dMinX, pMinItem->m_dMinY, pMinItem->m_dMaxX, pMinItem->m_dMaxY))
		{
			delete(pMinItem);
			delete(pRange);
			return false;
		}

		pMinItem->m_dMinX -= dTol;
		pMinItem->m_dMinY -= dTol;
		pMinItem->m_dMaxX += dTol;
		pMinItem->m_dMaxY += dTol;

		m_mapDouble2Item.insert(std::make_pair(pMinItem->m_dMinX, pMinItem));


		RangeItem* pMaxItem = new RangeItem(ipItem, pRange, false, isSrc);
		pMaxItem->m_dMinX = pMinItem->m_dMinX;
		pMaxItem->m_dMinY = pMinItem->m_dMinY;
		pMaxItem->m_dMaxX = pMinItem->m_dMaxX;
		pMaxItem->m_dMaxY = pMinItem->m_dMaxY;

		m_mapDouble2Item.insert(std::make_pair(pMinItem->m_dMaxX, pMaxItem));

		return true;
	}

	// SetItems时要设置误差
	void Range2d::GetIntersectItems(std::vector<RangeItem*>& vecIntersect, SrcDestFunction function)
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
				// map.Max.y >= src.Min.y 为了支持包含的情况：例：1米的桥架包含0.5米的桥架内回路
				std::multimap<double, BoundItem*, DoubleLess>::iterator it = mapY2Item.lower_bound(pSrcItem->m_dMinY);

				for (; it != mapY2Item.end(); ++it)
				{
					RangeItem* pDestItem = static_cast<RangeItem*>(it->second);

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

	// 勿用，此函数有待处理20200220
	void Range2d::GetSameItems(std::vector<RangeItem*>& vecIntersect)
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
				std::multimap<double, BoundItem*, DoubleLess>::iterator it = mapY2Item.lower_bound(pSrcItem->m_dMinY);

				for (; it != mapY2Item.end(); ++it)
				{
					RangeItem* pDestItem = static_cast<RangeItem*>(it->second);

					if (!pSrcItem->m_isSrc && !pDestItem->m_isSrc)
						continue;

					if (pDestItem->m_dMinY > pSrcItem->m_dMaxY)
						break;

					if (pDestItem->m_dMaxY <= pSrcItem->m_dMaxY)
					{
						pSrcItem->m_pItems->m_items.push_back(pDestItem);
						pDestItem->m_pItems->m_items.push_back(pSrcItem);
					}
				}

				// 此容器的Key是MinY
				mapY2Item.insert(std::make_pair(pSrcItem->m_dMinY, pSrcItem));
			}
			else // 最大点
			{
				// 取到当前Y
				PairMapIter pairIter = mapY2Item.equal_range(pSrcItem->m_dMinY);

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

} // namespace Sindy
