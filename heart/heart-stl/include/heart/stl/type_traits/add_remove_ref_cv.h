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
	template <typename... Ts>
	using void_t = void;

	template <typename T>
	struct remove_cv
	{
		using type = T;
	};

	template <typename T>
	struct remove_cv<const T>
	{
		using type = T;
	};

	template <typename T>
	struct remove_cv<volatile T>
	{
		using type = T;
	};

	template <typename T>
	struct remove_cv<const volatile T>
	{
		using type = T;
	};

	template <typename T>
	using remove_cv_t = typename remove_cv<T>::type;

	template <typename T>
	struct remove_reference
	{
		using type = T;
	};

	template <typename T>
	struct remove_reference<T&>
	{
		using type = T;
	};

	template <typename T>
	struct remove_reference<T&&>
	{
		using type = T;
	};

	template <typename T>
	using remove_reference_t = typename remove_reference<T>::type;

	template <typename T>
	using remove_cvref_t = remove_cv_t<remove_reference_t<T>>;

	template <typename T, typename = void>
	struct _add_reference
	{
		using l_value = T;
		using r_value = T;
	};

	template <typename T>
	struct _add_reference<T, void_t<T&>>
	{
		using l_value = T&;
		using r_value = T&&;
	};

	template <typename T>
	struct add_lvalue_reference
	{
		using type = typename _add_reference<T>::l_value;
	};

	template <typename T>
	using add_lvalue_reference_t = typename _add_reference<T>::l_value;

	template <typename T>
	struct add_rvalue_reference
	{
		using type = typename _add_reference<T>::r_value;
	};

	template <typename _Ty>
	using add_rvalue_reference_t = typename _add_reference<_Ty>::r_value;
}
