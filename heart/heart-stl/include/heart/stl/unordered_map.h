#pragma once

#include <heart/config.h>
#include <heart/types.h>

#include <boost/unordered/unordered_map.hpp>

namespace hrt
{
	using namespace boost::unordered;

	template <typename K, typename V, template <class B> class AllocT = std::allocator>
	using unordered_map_a = boost::unordered_map<K, V, boost::hash<K>, std::equal_to<K>, AllocT<std::pair<const K, V>>>;
}
