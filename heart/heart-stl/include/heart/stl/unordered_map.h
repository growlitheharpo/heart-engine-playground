#pragma once

#pragma warning(push)
#pragma warning(disable : 4996 26451 26110 28251 26495)

#include <heart/config.h>
#include <heart/types.h>

#include <heart/stl/allocator.h>

#include <boost/unordered/unordered_map.hpp>

namespace hrt
{
	using namespace boost::unordered;

	template <typename K, typename V, template <class B> class AllocT = hrt::allocator>
	using unordered_map_a = boost::unordered_map<K, V, boost::hash<K>, std::equal_to<K>, AllocT<std::pair<const K, V>>>;
}

#pragma warning(pop)
