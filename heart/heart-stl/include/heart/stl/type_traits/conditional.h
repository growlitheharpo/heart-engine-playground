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
	template <bool b, class T, class U>
	struct conditional
	{
		typedef T type;
	};

	template <class T, class U>
	struct conditional<false, T, U>
	{
		typedef U type;
	};

	template <bool b, class T, class U>
	using conditional_t = typename conditional<b, T, U>::type;
}
