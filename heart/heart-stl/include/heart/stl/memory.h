#pragma once

#include <heart/config.h>

#include <heart/stl/forward.h>

#include <memory>

namespace hrt
{
	template <typename T>
	using shared_ptr = std::shared_ptr<T>;

	template <typename T>
	using weak_ptr = std::weak_ptr<T>;

	template <typename T, typename... A>
	auto make_shared(A&&... args)
	{
		return std::make_shared<T>(hrt::forward<A>(args)...);
	}
}
