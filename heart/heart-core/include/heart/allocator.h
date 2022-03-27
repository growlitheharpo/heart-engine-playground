#pragma once

#include <heart/copy_move_semantics.h>
#include <heart/stl/util/canonical_typedefs.h>
#include <heart/types.h>

template <typename V>
struct HeartBaseAllocator
{
	DECLARE_STANDARD_TYPEDEFS(V);
	using void_pointer = void*;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	HeartBaseAllocator() = default;
	USE_DEFAULT_COPY_SEMANTICS(HeartBaseAllocator);
	virtual ~HeartBaseAllocator() = default;

	pointer address(reference r)
	{
		return &r;
	}

	const_pointer address(const_reference r)
	{
		return &r;
	}

	void construct(pointer p, const_reference val)
	{
		new ((V*)p) V(val);
	}

	void destroy(pointer p)
	{
		p->~V();
	}

	size_type max_size() const noexcept
	{
		return static_cast<size_t>(-1) / sizeof(value_type);
	}
};

// Default allocator that uses the global new and delete operators.
// Provides an example of how to make a subclass of HeartBaseAllocator
// fully std-compliant.
template <typename T>
struct HeartDefaultAllocator : public HeartBaseAllocator<T>
{
	HeartDefaultAllocator() = default;
	USE_DEFAULT_COPY_SEMANTICS(HeartDefaultAllocator);
	USING_STANDARD_TYPEDEFS(HeartBaseAllocator<T>);
	using size_type = HeartBaseAllocator<T>::size_type;
	using difference_type = HeartBaseAllocator<T>::difference_type;

	template <typename U>
	HeartDefaultAllocator(const HeartDefaultAllocator<U>&)
	{
	}

	pointer allocate(size_type n, void* hint = nullptr)
	{
		void* ptr = operator new(n * sizeof(value_type));
		return reinterpret_cast<pointer>(ptr);
	}

	void deallocate(pointer p, size_type n = 1)
	{
		operator delete(p, n * sizeof(value_type));
	}
};

using HeartDefaultByteAllocator = HeartDefaultAllocator<byte_t>;
