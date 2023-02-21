#pragma once
#include "../SindyGlobal.h"
#include "GeoClass.h"

namespace Sindy
{
#define ZERO 0.000001
#define UNKNOWN_REGION_ID -999999

	// <
	struct SINDY_API DoubleLess
	{
		inline bool operator()(const double& _Left, const double& _Right) const
		{
			if (fabs(_Left - _Right) < ZERO)
				return false;

			return _Left < _Right;
		}
	};

	typedef bool (*SrcDestFunction)(bool isLeftSrc, bool isRightSrc);

	// 所有
	inline bool CompareAll(bool, bool)
	{
		return true;
	}
	// 至少有一个是Src
	inline bool CompareSrc(bool isLeftSrc, bool isRightSrc)
	{
		if (isLeftSrc || isRightSrc)
			return true;
		return false;
	}
	// Src-Dest或Dest-Src
	inline bool CompareSrcDest(bool isLeftSrc, bool isRightSrc)
	{
		if (isLeftSrc != isRightSrc)
			return true;
		return false;
	}

	class IBoundItem
	{
	public:
		virtual bool GetId(REGIONID& id);
		virtual bool GetExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY);
	};

	// 这个class很像IBoundItem的实例
	class SINDY_API BoundItem
	{
	public:
		BoundItem(IBoundItem* ipItem);
		virtual ~BoundItem();

		// 以内嵌的方式包含IBoundItem，方便调用者转换
		IBoundItem* m_ipItem;
		double m_dMinX;
		double m_dMinY;
		double m_dMaxX;
		double m_dMaxY;
	};

	class SINDY_API RangeItem : public BoundItem
	{
	public:
		// 这个类是为了让同一Range的起点端、终点端共用同一个Ranges
		class Ranges
		{
		public:
			std::vector<RangeItem*> m_items;  // 这里不释放
		};

		RangeItem(IBoundItem* ipItem, Ranges* pRange, bool isMin, bool isSrc);
		~RangeItem();

		bool m_isMin; // 起点端标志
		bool m_isSrc; // 源 的标志

		// 范围内的其它Bound
		Ranges* m_pItems;

	public:

		// 简化客户代码
		std::vector<RangeItem*>::iterator Begin() { return m_pItems->m_items.begin(); }
		std::vector<RangeItem*>::iterator End() { return m_pItems->m_items.end(); }
	};


	// 获取某个范围内的实体
	class SINDY_API Range2d
	{
	public:
		~Range2d();
		void Reset();

		// 请调用者保证Item唯一性
		bool SetItemMin(IBoundItem* ipItem, double dTol = 0.0);
		// 获取某个范围内的Item
		void GetSameItem(const Point3d& ptMin, const Point3d& ptMax, std::set<IBoundItem*>& setRepeat, double dTol = 1000)const;
		void GetSameItem(const Point3d& ptInsert, std::set<IBoundItem*>& setRepeat, double radius, double dTol = 1000)const;

		// 请调用者保证Item唯一性
		bool SetItemMax(IBoundItem* ipItem, double dTol = 0.0);
		void GetIntersectItem(const Point3d& ptMin, const Point3d& ptMax, std::set<IBoundItem*>& setRepeat, double dTol)const;
		void GetIntersectItem(const Point3d& ptInsert, std::set<IBoundItem*>& setRepeat, double radius, double dTol = 1000)const;

		// 只输出源实体相关的Bound，请调用者保证Item唯一性
		bool SetItems(IBoundItem* ipItem, bool isSrc = true, double dTol = 0.0);
		// 获取相交的Item，包括覆盖的情况。调用者不要释放传出的容器
		void GetIntersectItems(std::vector<RangeItem*>& vecIntersect, SrcDestFunction function = CompareSrc);

	private:

		std::multimap<double, BoundItem*, DoubleLess> m_mapDouble2Item;
	};

	template<typename Array>
	void SetRangeItems(Range2d& range, const Array& arr, bool isSrc, double dTol)
	{
		for (const auto& item : arr)
		{
			range.SetItems(item, isSrc, dTol);
		}
	}

} // namespace Sindy