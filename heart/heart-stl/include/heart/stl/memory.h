#pragma once

#include <heart/config.h>

#include <boost/intrusive_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace hrt
{
	template <typename T>
	using intrusive_ptr = boost::intrusive_ptr<T>;

	template <typename T>
	using shared_ptr = boost::shared_ptr<T>;

	template <typename T>
	using weak_ptr = boost::weak_ptr<T>;
}
