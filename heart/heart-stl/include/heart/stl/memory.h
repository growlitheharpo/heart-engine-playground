#pragma once

#include <heart/config.h>

#include <memory>

namespace hrt
{
	template <typename T>
	using shared_ptr = std::shared_ptr<T>;

	template <typename T>
	using weak_ptr = std::weak_ptr<T>;
}
