#pragma once

namespace hrt
{
	template <bool b, class T, class U>
	struct conditional
	{
		typedef T type;
	};

	template <class T, class U>
	struct conditional<false, T, U>
	{
		typedef U type;
	};

	template <bool b, class T, class U>
	using conditional_t = typename conditional<b, T, U>::type;
}
