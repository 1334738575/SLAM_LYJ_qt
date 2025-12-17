#include "QT_LYJ_Defines.h"

NSP_QT_LYJ_BEGIN

QT_LYJ_API int64_t imagePair2Int64(int _i1, int _i2)
{
	if (_i1 <= _i2)
		return (static_cast<int64_t>(_i1) << 32) | static_cast<int64_t>(_i2);
	else
		return (static_cast<int64_t>(_i2) << 32) | static_cast<int64_t>(_i1);
}
QT_LYJ_API std::pair<int, int> int642TwoImagePair(int64_t _pair)
{
	int i1 = static_cast<int>(_pair >> 32);
	int i2 = static_cast<int>(_pair & 0xFFFFFFFF);
	return { i1, i2 };
}

NSP_QT_LYJ_END
