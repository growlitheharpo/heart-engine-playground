#pragma once

#include <heart/copy_move_semantics.h>
#include <heart/stl/forward.h>
#include <heart/stl/util/canonical_typedefs.h>
#include <heart/types.h>

/// <summary>
/// Base allocator type which is designed to be roughly std-compliant and as flexible as possible
/// Child classes must implement RawAllocate() and RawDeallocate()
/// </summary>
struct HeartBaseAllocator
{
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef void* void_pointer;

	HeartBaseAllocator() = default;
	DISABLE_COPY_SEMANTICS(HeartBaseAllocator);
	USE_DEFAULT_MOVE_SEMANTICS(HeartBaseAllocator);
	virtual ~HeartBaseAllocator() = default;

	virtual void* RawAllocate(size_type size, void* hint = 0) = 0;

	virtual void RawDeallocate(void* ptr, size_type n = 1) = 0;

	template <typename T, typename... Args>
	T* AllocateAndConstruct(Args&&... a)
	{
		T* p = allocate<T>(1);
		new (p) T(hrt::forward<Args>(a)...);
		return p;
	}

	template <typename T>
	void DestroyAndFree(T* p)
	{
		destroy<T>(p);
		deallocate(p);
	}

	template <typename T>
	T* allocate(size_type n, void* hint = nullptr)
	{
		void* ptr = RawAllocate(n * sizeof(T));
		return reinterpret_cast<T*>(ptr);
	}

	template <typename T>
	void deallocate(T* p, size_type n = 1)
	{
		RawDeallocate((void*)p, n);
	}

	// std allocator requirements
	template <typename T>
	T* address(T& r)
	{
		return &r;
	}

	template <typename T>
	const T* address(const T& r)
	{
		return &r;
	}

	template <typename T>
	void construct(T* p, const T& val)
	{
		new ((T*)p) T(val);
	}

	template <typename T>
	void destroy(T* p)
	{
		p->~T();
	}

	template <typename T>
	size_type max_size() const noexcept
	{
		return static_cast<size_t>(-1) / sizeof(T);
	}
};

/// <summary>
/// Simplest implementation of HeartBaseAllocator which uses global new and delete
/// </summary>
struct HeartDefaultAllocator final : public HeartBaseAllocator
{
	using size_type = HeartBaseAllocator::size_type;
	using difference_type = HeartBaseAllocator::difference_type;
	using void_pointer = HeartBaseAllocator::void_pointer;

	void* RawAllocate(size_type n, void* hint = nullptr) override
	{
		void* ptr = operator new(n);
		return ptr;
	}

	void RawDeallocate(void* p, size_type n = 1) override
	{
		operator delete(p);
	}
};

/// <summary>
/// Version of HeartBaseAllocator which moves the template "responsibility" from the
/// individual members to the type itself. Allows for use with std containers.
/// Requires child class to implement RawAllocate and RawDeallocate.
/// </summary>
template <typename T>
struct HeartBaseTypedAllocator : public HeartBaseAllocator
{
	using size_type = HeartBaseAllocator::size_type;
	using difference_type = HeartBaseAllocator::difference_type;
	DECLARE_STANDARD_TYPEDEFS(T);

	HeartBaseTypedAllocator() = default;
	virtual ~HeartBaseTypedAllocator() = default;

	pointer allocate(size_type n, void* hint = nullptr)
	{
		return HeartBaseAllocator::allocate<T>(n, hint);
	}

	void deallocate(pointer p, size_type n = 1)
	{
		HeartBaseAllocator::deallocate<T>(p, n);
	}

	pointer address(reference r)
	{
		return HeartBaseAllocator::address<T>(r);
	}

	const_pointer address(const_reference r)
	{
		return HeartBaseAllocator::address<T>(r);
	}

	void construct(pointer p, const_reference val)
	{
		HeartBaseAllocator::construct<T>(p, val);
	}

	void destroy(pointer p)
	{
		HeartBaseAllocator::destroy<T>(p);
	}

	size_type max_size() const noexcept
	{
		return HeartBaseAllocator::max_size<T>();
	}
};

/// <summary>
/// Simplest implementation of HeartBaseTypedAllocator which uses global new and delete
/// </summary>
template <typename T>
struct HeartDefaultTypedAllocator final : public HeartBaseTypedAllocator<T>
{
	using size_type = HeartBaseAllocator::size_type;
	using difference_type = HeartBaseAllocator::difference_type;
	USING_STANDARD_TYPEDEFS(HeartBaseTypedAllocator<T>);

	HeartDefaultTypedAllocator() = default;
	~HeartDefaultTypedAllocator() = default;

	void* RawAllocate(size_type n, void* hint = nullptr) override
	{
		return m_internalAllocator.RawAllocate(n, hint);
	}

	void RawDeallocate(void* p, size_type n = 1) override
	{
		m_internalAllocator.RawDeallocate(p, n);
	}

private:
	HeartDefaultAllocator m_internalAllocator = {};
};

HeartDefaultAllocator& GetHeartDefaultAllocator();
