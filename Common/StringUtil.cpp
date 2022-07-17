#include "StringUtil.h"





namespace Sindy
{
	int CompareDbl(double src1, double src2, double dTol)
	{
		double dblSub = src1 - src2;
		// 相同
		if (GetAbs(dblSub) <= dTol)
			return 0;
		else if (dblSub > dTol)
			return 1;
		else
			return -1;
	}

} // namespace Sindy