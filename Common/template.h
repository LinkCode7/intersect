#pragma once





namespace Sindy
{
	// 释放STL容器，不关心容器的内部结构
	template<typename Container>
	inline void ReleaseContainer(Container& list)
	{
		for (auto& item : list)
			delete item;
		list.clear();
	}

	template<typename Key, typename Value>
	inline void ReleaseMapValue(std::map<Key, Value*>& map)
	{
		for (auto& item : map)
			delete (item.second);
		map.clear();
	}

} // namespace Sindy
