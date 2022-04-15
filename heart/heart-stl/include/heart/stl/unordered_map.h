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

#include <heart/config.h>
#include <heart/types.h>

#include <heart/stl/allocator.h>

#include <unordered_map>

namespace hrt
{
	template <typename K, typename V, typename Hash = std::hash<K>, typename Eql = std::equal_to<K>, typename Alloc = std::allocator<std::pair<const K, V>>>
	using unordered_map = std::unordered_map<K, V, Hash, Eql, Alloc>;

	template <typename K, typename V, template <class B> class AllocT = hrt::allocator>
	using unordered_map_a = std::unordered_map<K, V, std::hash<K>, std::equal_to<K>, AllocT<std::pair<const K, V>>>;
}
