#pragma once
#include <string>



#define SINDY_API

namespace Sindy
{
	inline double getAbs(double dValue)
	{
		if (dValue > 0)
			return dValue;
		else
			return -dValue;
	}

	// 比较两个浮点数：0表示相同 1表示前面大 -1表示后面大
	SINDY_API int compareDbl(double src1, double src2, double dTol = 0.000001);

	template<typename Container>
	void joinStr(const Container& container, std::string& str, const std::string& split = ",")
	{
		if (container.empty())
			return;

		for (const auto& item : container)
		{
			str += item;
			str += split;
		}
		str.erase(str.end() - 1);
	}

} // namespace Sindy