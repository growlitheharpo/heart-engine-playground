#pragma once

#include <heart/config.h>
#include <heart/types.h>

#include <heart/allocator.h>
#include <heart/copy_move_semantics.h>
#include <heart/util/canonical_typedefs.h>

#include <heart/stl/forward.h>
#include <heart/stl/move.h>
#include <heart/stl/type_traits/constructible.h>

namespace heart_priv
{
	// Currently in heart_priv because it is massively lacking in functionality

	template <typename T>
	class HeartVector
	{
	public:
		DECLARE_STANDARD_TYPEDEFS(T);

	private:
		pointer m_buffer = nullptr;
		size_t m_size = 0;
		size_t m_capacity = 0;

		HeartBaseAllocator& m_allocator;

		pointer Reallocate(size_t newCapacity)
		{
			size_t originalCap = m_capacity;
			pointer originalData = m_buffer;
			pointer originalDataEnd = m_buffer + m_size;

			m_capacity = newCapacity;
			m_buffer = m_allocator.allocate<value_type>(m_capacity, originalData);

			pointer target = m_buffer;
			pointer newEnd = target + m_capacity;
			for (auto iter = originalData; iter < originalDataEnd && target < newEnd; ++iter, ++target)
			{
				if constexpr (hrt::is_move_constructible_v<value_type>)
				{
					m_allocator.construct<value_type>(target, hrt::move(*iter));
				}
				else
				{
					m_allocator.construct<value_type>(target, *iter);
				}

				m_allocator.destroy<value_type>(iter);
			}

			if (originalData != nullptr)
			{
				m_allocator.deallocate<value_type>(originalData, originalCap);
			}

			return pointer(m_buffer + originalCap);
		}

		pointer PushBackGetLocation()
		{
			pointer newLocation;

			if (m_capacity == 0)
				newLocation = Reallocate(1);
			else if (m_size < m_capacity)
				newLocation = m_buffer + m_size;
			else
				newLocation = Reallocate(m_capacity * 2);

			++m_size;
			return newLocation;
		}

	public:
		HeartVector(HeartBaseAllocator& alloc) :
			m_allocator(alloc),
			m_buffer(nullptr),
			m_capacity(0),
			m_size(0)
		{
		}

		~HeartVector()
		{
			Clear();
		}

		DISABLE_COPY_AND_MOVE_SEMANTICS(HeartVector);

		void Clear()
		{
			if (IsEmpty())
				return;

			for (size_t i = 0; i < m_size; ++i)
			{
				m_allocator.destroy<T>(m_buffer + i);
			}

			m_allocator.deallocate<T>(m_buffer, m_size);
			m_buffer = nullptr;
			m_size = 0;
			m_capacity = 0;
		}

		void Reserve(size_t count)
		{
			if (count <= m_size)
			{
				for (pointer iter = m_buffer + m_size - 1; iter != m_buffer + count; --iter)
				{
					m_allocator.destroy<value_type>(iter);
					--m_size;
				}
			}

			Reallocate(count);
		}

		bool IsEmpty() const
		{
			return m_size == 0;
		}

		size_t Size() const
		{
			return m_size;
		}

		template <typename... Args>
		reference EmplaceBack(Args&&... a)
		{
			pointer location = PushBackGetLocation();
			m_allocator.construct<value_type>(location, hrt::forward<Args>(a)...);
			return *location;
		}

		pointer begin()
		{
			return m_buffer;
		}

		pointer end()
		{
			return m_buffer + m_size;
		}

		const_pointer begin() const
		{
			return m_buffer;
		}

		const_pointer end() const
		{
			return m_buffer + m_size;
		}
	};
}
