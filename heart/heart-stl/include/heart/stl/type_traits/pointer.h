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
