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

#define HEART_DECLARE_TAG_TYPE(ValueName) \
	struct ValueName##T                   \
	{                                     \
	};                                    \
	static constexpr ValueName##T ValueName = {}
