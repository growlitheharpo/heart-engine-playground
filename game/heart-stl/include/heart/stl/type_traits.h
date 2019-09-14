#pragma once

#include <heart/stl/util/config.h>

#if HEART_IS_STD
#include <type_traits>
#endif

namespace hrt
{
#if HEART_IS_STD
	using namespace std;
#else
	template <typename T, T val>
	struct integral_constant
	{
		using value_type = T;
		using type = integral_constant;

		static constexpr value_type value = val;
		constexpr operator value_type() const noexcept { return value; }
		constexpr value_type operator()() const noexcept { return value; }
	};

	template <bool val>
	using bool_constant = integral_constant<bool, val>;

	using true_type = bool_constant<true>;
	using false_type = bool_constant<false>;

	template <class T>
	struct remove_reference 
	{
		using type = T;
	};

	template <class T>
	struct remove_reference<T&> 
	{
		using type = T;
	};

	template <class T>
	struct remove_reference<T&&> 
	{
		using type = T;
	};

	template <class T>
	using remove_reference_t = typename remove_reference<T>::type;

	template <class T>
	constexpr T&& forward(remove_reference_t<T>& arg) noexcept 
	{
		return static_cast<T&&>(arg);
	}

	template <class T>
	constexpr T&& forward(remove_reference_t<T>&& arg) noexcept 
	{
		return static_cast<T&&>(arg);
	}

	template <class T>
	constexpr remove_reference_t<T>&& move(T&& _Arg) noexcept 
	{
		return static_cast<remove_reference_t<T>&&>(_Arg);
	}
#endif
}
