#pragma once

#include <heart/config.h>

#include <functional>

namespace hrt
{
	template <typename... T>
	using bind = std::bind<T...>;

	template <typename... T>
	using function = std::function<T...>;
}
