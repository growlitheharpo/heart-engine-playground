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
#include <heart/types.h>

#include <utility>

namespace hrt
{
	template <typename... Tys>
	using tuple = std::tuple<Tys...>;

	template <typename T1, typename T2>
	using pair = std::pair<T1, T2>;

	template <typename T1, typename T2>
	auto make_pair(T1&& a, T2&& b)
	{
		return std::make_pair(a, b);
	}

	template <auto N, typename T>
	const auto& get(const T& v)
	{
		return std::get<N>(v);
	}

	template <size_t N, typename T>
	auto& get(T& v)
	{
		return std::get<N>(v);
	}

	template <auto N, typename T>
	auto&& get(T&& v)
	{
		return std::get<N>(v);
	}
}
