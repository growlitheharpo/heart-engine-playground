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

namespace hrt
{
	template <class>
	constexpr bool is_pointer_v = false;

	template <class T>
	constexpr bool is_pointer_v<T*> = true;

	template <class T>
	constexpr bool is_pointer_v<T* const> = true;

	template <class T>
	constexpr bool is_pointer_v<T* volatile> = true;

	template <class T>
	constexpr bool is_pointer_v<T* const volatile> = true;

	template <class T>
	constexpr T* addressof(T& v) noexcept
	{
		return __builtin_addressof(v);
	}

	template <class T>
	const T* addressof(const T&&) = delete;
}
