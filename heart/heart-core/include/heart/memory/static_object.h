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

#include <heart/types.h>

#include <heart/stl/forward.h>
#include <heart/stl/type_traits/aligned_storage.h>
#include <heart/stl/type_traits/constants.h>
#include <heart/util/canonical_typedefs.h>

// HeartStaticObject allows you to use static storage for an object which you still
// need to control the lifetime over. It is essentially just a std::optional.
// If StaticConstruct is set to true, construction will be done during static initialization
// but you can still control the destruction time (and re-construct/destruct as needed).
using HeartStaticObjectEnableStaticConstruct = hrt::true_type;
using HeartStaticObjectDisableStaticConstruct = hrt::false_type;

template <typename T, typename ShouldStaticConstruct = HeartStaticObjectDisableStaticConstruct>
class HeartStaticObject
{
	static_assert(
		hrt::is_same_v<ShouldStaticConstruct, HeartStaticObjectEnableStaticConstruct> ||
			hrt::is_same_v<ShouldStaticConstruct, HeartStaticObjectDisableStaticConstruct>,
		"Second template argument should be either HeartStaticObjectEnableStaticConstruct or HeartStaticObjectDisableStaticConstruct");

public:
	DECLARE_STANDARD_TYPEDEFS(T);

	template <typename... Args>
	HeartStaticObject(Args... args)
	{
		if constexpr (hrt::is_same_v<ShouldStaticConstruct, HeartStaticObjectEnableStaticConstruct>)
		{
			Construct(hrt::forward<Args>(args)...);
		}
	}

	~HeartStaticObject()
	{
		Destruct();
	}

	template <typename... Args>
	void Construct(Args... args)
	{
		if (m_initialized)
			return;

		pointer p = reinterpret_cast<pointer>(&storage);
		p = new (p) value_type(hrt::forward<Args>(args)...);
		m_initialized = true;
	}

	void Destruct()
	{
		if (!m_initialized)
			return;

		pointer p = reinterpret_cast<pointer>(&storage);
		p->~value_type();
		m_initialized = false;
	}

	bool IsInitialized() const
	{
		return m_initialized;
	}

	reference Get()
	{
		return *reinterpret_cast<pointer>(&storage);
	}

	const_reference Get() const
	{
		return *reinterpret_cast<const_pointer>(&storage);
	}

	pointer operator->()
	{
		return reinterpret_cast<pointer>(&storage);
	}

	const_pointer operator->() const
	{
		return reinterpret_cast<const_pointer>(&storage);
	}

	reference operator*()
	{
		return Get();
	}

	const_reference operator*() const
	{
		return Get();
	}

private:
	bool m_initialized = false;

	using Storage = hrt::aligned_storage_t<sizeof(value_type)>;
	Storage storage = {};
};
