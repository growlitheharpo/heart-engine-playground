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
