#pragma once





namespace Sindy
{
	// 释放STL容器，不关心容器的内部结构
	template<typename Container>
	inline void ReleaseContainer(Container& list)
	{
		Container::iterator iter = list.begin();
		Container::iterator theEnd = list.end();
		for (; iter != theEnd; ++iter)
		{
			delete (*iter);
		}
		list.clear();
	}

	template<typename Key, typename Value>
	inline void ReleaseMapValue(std::map<Key, Value*>& map)
	{
		std::map<Key, Value*>::iterator theIter = map.begin();
		for (; theIter != map.end(); ++theIter)
		{
			delete (theIter->second);
		}
		map.clear();
	}

} // namespace Sindy
