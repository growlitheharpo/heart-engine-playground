#pragma once

#include <heart/stl/forward.h>
#include <heart/stl/move.h>
#include <heart/stl/type_traits/add_remove_ref_cv.h>
#include <heart/stl/type_traits/same.h>

namespace heart_priv
{
	template <typename R, typename... Args>
	class HeartFunctionBase
	{
	public:
		virtual R Call(Args&&... args) = 0;
		virtual void Move(void* location) = 0;
		virtual ~HeartFunctionBase() = default;
	};

	template <typename F, typename R, typename... Args>
	class HeartFunctionImpl : public HeartFunctionBase<R, Args...>
	{
		static_assert(hrt::is_same_v<F, hrt::remove_reference_t<F>>);
		F f;

	public:
		HeartFunctionImpl(F&& f) :
			f(hrt::forward<F>(f))
		{
		}

		~HeartFunctionImpl() = default;

		R Call(Args&&... args) override
		{
			return f(hrt::forward<Args>(args)...);
		}

		void Move(void* location) override
		{
			new (location) HeartFunctionImpl<F, R, Args...>(hrt::move(f));
		}
	};
}
