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
#include <heart/copy_move_semantics.h>
#include <heart/types.h>

#include <heart/util/canonical_typedefs.h>

#include <type_traits>

struct HeartIntrusiveListLink
{
	HeartIntrusiveListLink* prev = nullptr;
	HeartIntrusiveListLink* next = nullptr;
};

template <typename T>
struct HeartIntrusiveListIterator;

template <typename T>
struct HeartIntrusiveListConstIterator;

template <typename T, HeartIntrusiveListLink T::*LinkPointer>
class HeartIntrusiveList
{
public:
	DECLARE_STANDARD_TYPEDEFS(T);
	using iterator = HeartIntrusiveListIterator<HeartIntrusiveList<T, LinkPointer>>;
	using const_iterator = HeartIntrusiveListConstIterator<HeartIntrusiveList<T, LinkPointer>>;
	using link_type = HeartIntrusiveListLink;
	typedef link_type value_type::*link_pointer_t;

	friend struct iterator;
	friend struct const_iterator;

public:
	HeartIntrusiveList()
	{
		m_root.next = &m_root;
		m_root.prev = &m_root;
		m_size = 0;
	}

	DISABLE_COPY_SEMANTICS(HeartIntrusiveList);

	HeartIntrusiveList(HeartIntrusiveList&& o) :
		HeartIntrusiveList()
	{
		AddLinkBetweenLinks(&m_root, &o.m_root, o.m_root.next);
		RemoveLink(&o.m_root);
		m_size = o.m_size;

		o.m_root.next = &o.m_root;
		o.m_root.prev = &o.m_root;
		o.m_size = 0;
	}

	HeartIntrusiveList& operator=(HeartIntrusiveList&& o)
	{
		// Should this be an error if we're not empty?
		Clear();

		AddLinkBetweenLinks(&m_root, &o.m_root, o.m_root.next);
		RemoveLink(&o.m_root);
		m_size = o.m_size;

		o.m_root.next = &o.m_root;
		o.m_root.prev = &o.m_root;
		o.m_size = 0;

		return *this;
	}

	void PushFront(pointer p)
	{
		AddLinkBetweenLinks(LinkFromPointer(p), &m_root, m_root.next);
	}

	void PushBack(pointer p)
	{
		AddLinkBetweenLinks(LinkFromPointer(p), m_root.prev, &m_root);
	}

	pointer PopFront()
	{
		if (IsEmpty())
			return nullptr;

		pointer element = PointerFromLink(m_root.next);
		Remove(element);
		return element;
	}

	pointer PopBack()
	{
		if (IsEmpty())
			return nullptr;

		pointer element = PointerFromLink(m_root.prev);
		Remove(element);
		return element;
	}

	POINTER_AND_CONST_POINTER(Front(),
		{
			if (IsEmpty())
				return nullptr;

			return PointerFromLink(m_root.next);
		})

	POINTER_AND_CONST_POINTER(Back(),
		{
			if (IsEmpty())
				return nullptr;

			return PointerFromLink(m_root.prev);
		})

	void Remove(pointer p)
	{
		RemoveLink(LinkFromPointer(p));
	}

	void Remove(iterator iterator)
	{
		RemoveLink(iterator.link);
	}

	void Clear()
	{
		while (!IsEmpty())
		{
			RemoveLink(m_root.prev);
		}
	}

	bool Contains(pointer p) const
	{
		const link_type* link = m_root.next;
		while (link != &m_root)
		{
			if (PointerFromLink(link) == p)
				return true;

			link = link->next;
		}

		return false;
	}

	size_t Size() const
	{
		return m_size;
	}

	bool IsEmpty() const
	{
		return Size() == 0;
	}

	iterator begin()
	{
		return iterator(*this, m_root.next);
	}

	iterator end()
	{
		return iterator(*this, &m_root);
	}

	const_iterator begin() const
	{
		return const_iterator(*this, m_root.next);
	}

	const_iterator end() const
	{
		return const_iterator(*this, &m_root);
	}

private:
	static link_type* LinkFromPointer(pointer p)
	{
		return &(p->*LinkPointer);
	}

	pointer PointerFromLink(link_type* link) const
	{
		if (link == &m_root)
			return nullptr;

		return reinterpret_cast<pointer>(reinterpret_cast<char*>(link) - reinterpret_cast<ptrdiff_t>(&(reinterpret_cast<value_type const*>(NULL)->*LinkPointer)));
	}

	const_pointer PointerFromLink(const link_type* link) const
	{
		if (link == &m_root)
			return nullptr;

		return reinterpret_cast<const_pointer>(reinterpret_cast<const char*>(link) - reinterpret_cast<ptrdiff_t>(&(reinterpret_cast<value_type const*>(NULL)->*LinkPointer)));
	}

	void AddLinkBetweenLinks(link_type* item, link_type* prev, link_type* next)
	{
		item->next = next;
		item->prev = prev;
		next->prev = item;
		prev->next = item;
		++m_size;
	}

	void RemoveLink(link_type* item)
	{
		if (item == &m_root)
			return;

		item->next->prev = item->prev;
		item->prev->next = item->next;
		item->next = nullptr;
		item->prev = nullptr;
		--m_size;
	}

private:
	link_type m_root;
	size_t m_size;
};

template <typename ListType>
struct HeartIntrusiveListIterator
{
	using list_type = ListType;
	using link_type = list_type::link_type;
	using this_type = HeartIntrusiveListIterator<ListType>;
	USING_STANDARD_TYPEDEFS(list_type);

	list_type& list;
	link_type* link;

	HeartIntrusiveListIterator(list_type& l, link_type* p) :
		list(l),
		link(p)
	{
	}

	REF_AND_CONST_REF(operator*(),
		{
			return *list.PointerFromLink(link);
		})

	POINTER_AND_CONST_POINTER(operator->(),
		{
			return list.PointerFromLink(link);
		})

	friend bool operator==(const this_type& lhs, const this_type& rhs)
	{
		return lhs.link == rhs.link;
	}

	bool operator<(const this_type& rhs) const
	{
		const auto& lhs = *this;

		const link_type& root = lhs.list.m_root;
		const link_type* link = root.next;
		while (link != &root)
		{
			if (link == lhs.link)
				return true;

			if (link == rhs.link)
				return false;

			link = link->next;
		}

		return false;
	}

	this_type& operator++()
	{
		link = link->next;
		return *this;
	}

	this_type operator++(int)
	{
		this_type tmp(list, link);
		this->operator++();
		return tmp;
	}
};

template <typename ListType>
struct HeartIntrusiveListConstIterator
{
	using list_type = ListType;
	using link_type = list_type::link_type;
	using this_type = HeartIntrusiveListConstIterator<ListType>;
	USING_STANDARD_TYPEDEFS(list_type);

	const list_type& list;
	const link_type* link;

	HeartIntrusiveListConstIterator(const list_type& l, const link_type* p) :
		list(l),
		link(p)
	{
	}

	const_reference operator*() const
	{
		return *list.PointerFromLink(link);
	}

	const_pointer operator->() const
	{
		return list.PointerFromLink(link);
	}

	friend bool operator==(const this_type& lhs, const this_type& rhs)
	{
		return lhs.link == rhs.link;
	}

	bool operator<(const this_type& rhs) const
	{
		auto& lhs = *this;

		const link_type& root = lhs.list.m_root;
		const link_type* link = root.next;
		while (link != &root)
		{
			if (link == lhs.link)
				return true;

			if (link == rhs.link)
				return false;

			link = link->next;
		}

		return false;
	}

	this_type& operator++()
	{
		link = link->next;
		return *this;
	}

	this_type operator++(int)
	{
		this_type tmp(list, link);
		this->operator++();
		return tmp;
	}
};
