#pragma once

template <typename ArrT, decltype(sizeof(void*)) N>
auto HeartCountOf(ArrT (&X)[N])
{
	return N;
}
