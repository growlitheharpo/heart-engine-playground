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

/*
 * Loosely based on ScopeGuard.h as defined in:
 *   Alexandrescu, A., Marginean, P.:"Generic<Programming>: Change the Way You
 *     Write Exception-Safe Code - Forever", C/C++ Users Journal, Dec 2000,
 *     http://www.drdobbs.com/184403758
 * However, whereas that implementation is designed to (essentially)
 * std::bind its own functor, we only support being passed something
 * already callable with no arguments like a lambda.
 */

template <typename F>
class HeartScopeGuard
{
private:
	F func_;
	bool dismissed_ = false;

public:
	HeartScopeGuard(F&& f) :
		func_(f)
	{
	}

	~HeartScopeGuard()
	{
		if (!dismissed_)
			func_();
	}

	void Dismiss()
	{
		dismissed_ = true;
	}
};

#define ___HEART_SCOPE_EXIT(a, b) a##b
#define _HEART_SCOPE_EXIT(x, n) ___HEART_SCOPE_EXIT(x, n)
#define HEART_SCOPE_EXIT(x) auto _HEART_SCOPE_EXIT(HeartGuard_, __COUNTER__) = HeartScopeGuard(x)
