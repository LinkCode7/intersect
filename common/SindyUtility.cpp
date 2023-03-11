#include "SindyUtility.h"

int Sindy::compareDbl(double src1, double src2, double dTol)
{
	double dblSub = src1 - src2;
	// 相同
	if (getAbs(dblSub) <= dTol)
		return 0;
	else if (dblSub > dTol)
		return 1;
	else
		return -1;
}
