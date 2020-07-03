#pragma once

#include <heart/copy_move_semantics.h>
#include <heart/stl/util/canonical_typedefs.h>
#include <heart/types.h>

namespace Memory
{
	template <typename V>
	struct BaseAllocator
	{
		DECLARE_STANDARD_TYPEDEFS(V);
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		BaseAllocator() = default;
		USE_DEFAULT_COPY_SEMANTICS(BaseAllocator);
		virtual ~BaseAllocator() = default;

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
			new ((T*)p) T(val);
		}

		void destroy(pointer p)
		{
			p->~T();
		}
	};

	enum class Pool
	{
		UI,
		Debug,
		Generic,
		Unknown,

		Count,
	};

	enum class Period
	{
		Short,
		Long,
		Frame,

		Count,
	};

	void* Alloc(size_t size, Pool p, Period l);
	void Free(void* p);

	template <typename V, Pool p, Period t>
	struct BasePoolAllocator : public BaseAllocator<V>
	{
		BasePoolAllocator() = default;
		USE_DEFAULT_COPY_SEMANTICS(BasePoolAllocator);

		template <typename U>
		BasePoolAllocator(const BasePoolAllocator<U, p, t>&)
		{
		}

		pointer allocate(size_type count, void* hint = nullptr)
		{
			return pointer(Alloc(count * sizeof(V), p, t));
		}

		void deallocate(pointer p, size_type count = 0)
		{
			Free(p);
		}
	};

#define TYPEDEF_ALLOCATOR(P)                                                \
	template <typename V>                                                   \
	using P##ShortAllocator = BasePoolAllocator<V, Pool::P, Period::Short>; \
	template <typename V>                                                   \
	using P##LongAllocator = BasePoolAllocator<V, Pool::P, Period::Long>;   \
	template <typename V>                                                   \
	using P##FrameAllocator = BasePoolAllocator<V, Pool::P, Period::Frame>;

	TYPEDEF_ALLOCATOR(UI);
	TYPEDEF_ALLOCATOR(Debug);
	TYPEDEF_ALLOCATOR(Generic);

#undef TYPEDEF_ALLOCATOR

	void Init();

	void DebugDisplay();

	constexpr size_t Kilo = 1024;
	constexpr size_t Meg = Kilo * Kilo;
	constexpr size_t Gig = Meg * Kilo;
}
