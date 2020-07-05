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
