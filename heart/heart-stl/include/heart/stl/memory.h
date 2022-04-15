/* Copyright (C) 2022 James Keats
*
* This file is part of Heart, a collection of game engine technologies.
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
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
