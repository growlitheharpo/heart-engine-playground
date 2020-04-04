#pragma once

#include <heart/config.h>

namespace hrt
{
	template <class... Ts>
	using void_t = void;

	template <typename T, T val>
	struct integral_constant
	{
		using value_type = T;
		using type = integral_constant;

		static constexpr value_type value = val;
		constexpr operator value_type() const noexcept
		{
			return value;
		}

		constexpr value_type operator()() const noexcept
		{
			return value;
		}
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

	template <class T, class = void>
	struct _add_reference
	{
		using l_value = T;
		using r_value = T;
	};

	template <class T>
	struct _add_reference<T, void_t<T&>>
	{
		using l_value = T&;
		using r_value = T&&;
	};

	template <class T>
	struct add_lvalue_reference
	{
		using type = typename _add_reference<T>::l_value;
	};

	template <class T>
	using add_lvalue_reference_t = typename _add_reference<T>::l_value;

	template <class T>
	struct add_rvalue_reference
	{
		using type = typename _add_reference<T>::r_value;
	};

	template <class _Ty>
	using add_rvalue_reference_t = typename _add_reference<_Ty>::r_value;

	template <class T>
	add_rvalue_reference_t<T> declval() noexcept;

	// Boooo, having to use MSVC extensions for these...

	template <class T, class... Args>
	struct is_constructible : bool_constant<__is_constructible(T, Args...)>
	{
	};

	template <class T, class... Args>
	inline constexpr bool is_constructible_v = __is_constructible(T, Args...);

	template <class T>
	struct is_copy_constructible : bool_constant<__is_constructible(T, add_lvalue_reference_t<const T>)>
	{
	};

	template <class T>
	inline constexpr bool is_copy_constructible_v = __is_constructible(T, add_lvalue_reference_t<const T>);

	template <class T>
	struct is_default_constructible : bool_constant<__is_constructible(T)>
	{
	};

	template <class T>
	inline constexpr bool is_default_constructible_v = __is_constructible(T);

	template <class T>
	struct is_move_constructible : bool_constant<__is_constructible(T, T)>
	{
	};

	template <class T>
	inline constexpr bool is_move_constructible_v = __is_constructible(T, T);

	template <class _Ty>
	struct is_trivially_copyable : bool_constant<__is_trivially_copyable(_Ty)>
	{
	};

	template <class _Ty>
	inline constexpr bool is_trivially_copyable_v = __is_trivially_copyable(_Ty);

	template <typename T1, typename T2>
	inline constexpr bool is_same_v = false;

	template <typename T>
	inline constexpr bool is_same_v<T, T> = true;

	template <typename T1, typename T2>
	struct is_same : bool_constant<is_same_v<T1, T2>>
	{
	};
}
