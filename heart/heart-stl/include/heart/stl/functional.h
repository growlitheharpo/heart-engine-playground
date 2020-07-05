#pragma once

#include <heart/config.h>

#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace hrt
{
	template <typename... T>
	using bind = boost::bind<T...>;

	template <typename... T>
	using function = boost::function<T...>;
}
