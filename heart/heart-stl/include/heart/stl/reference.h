#pragma once

#include <heart/stl/forward.h>
#include <heart/stl/type_traits.h>

// TODO: get rid of this, use all hrt::
#include <type_traits>

namespace hrt
{
	namespace detail
	{
		template <typename T>
		constexpr T& wrap_ref(T& w) noexcept
		{
			return w;
		}

		template <typename T>
		void wrap_ref(T&&) = delete;
	}

	template <typename T>
	class reference_wrapper
	{
	private:
		T* m_ptr;

	public:
		typedef T type;

		// I have no idea, I copied it from cppreference.com...
		template <class U, class = decltype(
							   detail::wrap_ref<T>(declval<U>()),
							   enable_if_t<!is_same_v<reference_wrapper, remove_cvref_t<U>>>())>
		constexpr reference_wrapper(U&& u) noexcept(noexcept(detail::FUN<T>(forward<U>(u)))) :
			m_ptr(addressof(detail::wrap_ref<T>(forward<U>(u))))
		{
		}

		reference_wrapper(const reference_wrapper&) noexcept = default;

		constexpr operator T&() const noexcept
		{
			return *m_ptr;
		}

		constexpr T& get() const noexcept
		{
			return *m_ptr;
		}

		template <class... ArgTypes>
		constexpr invoke_result_t<T&, ArgTypes...>
		operator()(ArgTypes&&... args) const
		{
			return std::invoke(get(), forward<ArgTypes>(args)...);
		}
	};
}
