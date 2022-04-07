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

template <typename T, HeartIntrusiveListLink T::*LinkPointer>
class HeartIntrusiveList
{
public:
	DECLARE_STANDARD_TYPEDEFS(T);
	typedef HeartIntrusiveListLink value_type::*link_pointer_t;

	static_assert(std::is_standard_layout<value_type>::value);

public:
	struct HeartIntrusiveListIterator
	{
		HeartIntrusiveList& list;
		HeartIntrusiveListLink* link;

		HeartIntrusiveListIterator(HeartIntrusiveList& l, HeartIntrusiveListLink* p) :
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

		friend bool operator==(const HeartIntrusiveListIterator& lhs, const HeartIntrusiveListIterator& rhs)
		{
			return lhs.link == rhs.link;
		}

		bool operator<(const HeartIntrusiveListIterator& rhs) const
		{
			auto& lhs = *this;

			HeartIntrusiveListLink& root = lhs.list.m_root;
			HeartIntrusiveListLink* link = root.next;
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

		HeartIntrusiveListIterator& operator++()
		{
			link = link->next;
			return *this;
		}

		HeartIntrusiveListIterator operator++(int)
		{
			HeartIntrusiveListIterator tmp(list, link);
			this->operator++();
			return tmp;
		}
	};

public:
	HeartIntrusiveList()
	{
		m_root.next = &m_root;
		m_root.prev = &m_root;
		m_size = 0;
	}

	// TODO: This should be movable
	DISABLE_COPY_AND_MOVE_SEMANTICS(HeartIntrusiveList);

	void AddHead(pointer p)
	{
		AddLinkBetweenLinks(LinkFromPointer(p), &m_root, m_root.next);
	}

	void AddTail(pointer p)
	{
		AddLinkBetweenLinks(LinkFromPointer(p), m_root.prev, &m_root);
	}

	POINTER_AND_CONST_POINTER(GetHead(),
		{
			if (IsEmpty())
				return nullptr;

			return PointerFromLink(m_root.next);
		})

	POINTER_AND_CONST_POINTER(GetTail(),
		{
			if (IsEmpty())
				return nullptr;

			return PointerFromLink(m_root.prev);
		})

	void Remove(pointer p)
	{
		RemoveLink(LinkFromPointer(p));
	}

	void Remove(HeartIntrusiveListIterator iterator)
	{
		RemoveLink(iterator.link);
	}

	void Clear()
	{
		if (!IsEmpty())
		{
			HeartIntrusiveListLink* link = m_root.next;
			while (link != &m_root)
			{
				HeartIntrusiveListLink* safeNext = link->next;

				RemoveLink(link);

				link = safeNext;
			}
		}
	}

	size_t Size() const
	{
		return m_size;
	}

	bool IsEmpty() const
	{
		return Size() == 0;
	}

	HeartIntrusiveListIterator begin()
	{
		return HeartIntrusiveListIterator(*this, m_root.next);
	}

	HeartIntrusiveListIterator end()
	{
		return HeartIntrusiveListIterator(*this, &m_root);
	}

private:
	static HeartIntrusiveListLink* LinkFromPointer(pointer p)
	{
		return &(p->*LinkPointer);
	}

	pointer PointerFromLink(HeartIntrusiveListLink* link) const
	{
		if (link == &m_root)
			return nullptr;

		return reinterpret_cast<pointer>(reinterpret_cast<char*>(link) - reinterpret_cast<ptrdiff_t>(&(reinterpret_cast<value_type const*>(NULL)->*LinkPointer)));
	}

	const_pointer PointerFromLink(const HeartIntrusiveListLink* link) const
	{
		if (link == &m_root)
			return nullptr;

		return reinterpret_cast<const_pointer>(reinterpret_cast<const char*>(link) - reinterpret_cast<ptrdiff_t>(&(reinterpret_cast<value_type const*>(NULL)->*LinkPointer)));
	}

	void AddLinkBetweenLinks(HeartIntrusiveListLink* item, HeartIntrusiveListLink* prev, HeartIntrusiveListLink* next)
	{
		item->next = next;
		item->prev = prev;
		next->prev = item;
		prev->next = item;
		++m_size;
	}

	void RemoveLink(HeartIntrusiveListLink* item)
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
	HeartIntrusiveListLink m_root;
	size_t m_size;
};
