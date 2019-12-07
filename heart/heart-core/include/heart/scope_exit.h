#pragma once

#include <heart/stl/functional.h>

class HeartScopeExit
{
private:
	hrt::function<void()> target_;

public:
	HeartScopeExit(hrt::function<void()>&& f)
	{
		target_ = hrt::move(f);
	}

	~HeartScopeExit()
	{
		if (target_)
			target_();
	}
};

#define ___HEART_SCOPE_EXIT(a, b) a##b
#define _HEART_SCOPE_EXIT(x, n) ___HEART_SCOPE_EXIT(x, n)
#define HEART_SCOPE_EXIT(x) auto _HEART_SCOPE_EXIT(HeartGuard_, __COUNTER__) = HeartScopeExit(x)
