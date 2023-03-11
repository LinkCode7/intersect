#pragma once
#include "../SindyGlobal.h"
#include "../Common/SindyUtility.h"

#define ZERO 0.000001
#define UNKNOWN_REGION_ID -999999

namespace Sindy
{
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
	inline bool compareAll(bool, bool)
	{
		return true;
	}
	// 至少有一个是Src
	inline bool compareSrc(bool isLeftSrc, bool isRightSrc)
	{
		if (isLeftSrc || isRightSrc)
			return true;
		return false;
	}
	// Src-Dest或Dest-Src
	inline bool compareSrcDest(bool isLeftSrc, bool isRightSrc)
	{
		if (isLeftSrc != isRightSrc)
			return true;
		return false;
	}

	class SINDY_API IBoundItem
	{
	public:
		virtual bool getId(REGIONID& id);
		virtual bool getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY);
	};

	// 这个class很像IBoundItem的实例
	class SINDY_API BoundItem
	{
	public:
		BoundItem(IBoundItem* ipItem);
		virtual ~BoundItem() = default;

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
		struct Ranges
		{
			std::vector<RangeItem*> m_items;  // 这里不释放
		};

		RangeItem(IBoundItem* ipItem, Ranges* pRange, bool isMin, bool isSrc);
		~RangeItem();

		bool m_isMin; // 起点端标志
		bool m_isSrc; // 源 的标志
		Ranges* m_pItems; // 范围内的其它Bound

	public:

		// 简化客户代码
		std::vector<RangeItem*>::iterator begin() { return m_pItems->m_items.begin(); }
		std::vector<RangeItem*>::iterator end() { return m_pItems->m_items.end(); }
	};

	class SINDY_API Range2d
	{
	public:
		~Range2d();
		void reset();

		// 请调用者保证Item唯一性
		bool setItem(IBoundItem* ipItem, bool isSrc = true, double tol = 0.0);
		// 核心函数：获取相交的Item，包括覆盖的情况。只输出源实体相关的Bound，调用者不要释放传出的容器
		void getIntersectItem(std::vector<RangeItem*>& vecIntersect, SrcDestFunction function = compareSrc);

		template<typename Array>
		void setRangeItems(const Array& arr, bool isSrc, double tol);

	private:

		std::multimap<double, BoundItem*, DoubleLess> m_mapDouble2Item;
	};

	template<typename Array>
	void Range2d::setRangeItems(const Array& arr, bool isSrc, double tol)
	{
		for (const auto& item : arr)
		{
			setItem(item, isSrc, tol);
		}
	}

	/*
	* 以下为getIntersectItem的变体
	*/
	// 获取某个范围内的Item
	void getSameItem(const std::vector<IBoundItem*>& items, const Point3d& ptMin, const Point3d& ptMax, std::set<IBoundItem*>& setRepeat, double tol = 1.0);
	void getSameItem(const std::vector<IBoundItem*>& items, const Point3d& ptInsert, std::set<IBoundItem*>& setRepeat, double radius, double tol = 1.0);

	// 请调用者保证Item唯一性，变体
	void getIntersectItem2(const std::vector<IBoundItem*>& ipItem, const Point3d& ptMin, const Point3d& ptMax, std::set<IBoundItem*>& setRepeat, double tol = 1.0);

} // namespace Sindy