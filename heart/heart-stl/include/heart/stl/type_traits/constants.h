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
}
