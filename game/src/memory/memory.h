#pragma once

#include <heart/allocator.h>

#include <heart/stl/forward.h>
#include <heart/stl/unique_ptr.h>

namespace Memory
{
	enum class Pool
	{
		Events,
		UI,
		Debug,
		Generic,
		Fibers,
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

	void Free(void* ptr, Pool p, Period l);

	template <Pool pool, Period period>
	void* Alloc(size_t size)
	{
		return Alloc(size, pool, period);
	}

	template <Pool pool, Period period>
	void Free(void* ptr)
	{
		Free(ptr, pool, period);
	}

	template <typename V, Pool pool, Period period>
	struct BasePoolAllocator : public HeartBaseTypedAllocator<V>
	{
		static constexpr Pool TargetPool = pool;
		static constexpr Period TargetPeriod = period;

		BasePoolAllocator() = default;
		USE_DEFAULT_COPY_SEMANTICS(BasePoolAllocator);
		USING_STANDARD_TYPEDEFS(HeartBaseTypedAllocator<V>);
		using size_type = HeartBaseTypedAllocator<V>::size_type;
		using difference_type = HeartBaseTypedAllocator<V>::difference_type;

		template <typename U>
		BasePoolAllocator(const BasePoolAllocator<U, TargetPool, TargetPeriod>&)
		{
		}

		template <typename U>
		struct rebind
		{
			using other = BasePoolAllocator<U, TargetPool, TargetPeriod>;
		};

		void* RawAllocate(size_type bytes, void* hint = nullptr) override
		{
			return Alloc<TargetPool, TargetPeriod>(bytes);
		}

		void RawDeallocate(void* ptr, size_type n = 0) override
		{
			Free<TargetPool, TargetPeriod>(ptr);
		}
	};

#define TYPEDEF_ALLOCATOR(P)                                                \
	template <typename V>                                                   \
	using P##ShortAllocator = BasePoolAllocator<V, Pool::P, Period::Short>; \
	template <typename V>                                                   \
	using P##LongAllocator = BasePoolAllocator<V, Pool::P, Period::Long>;   \
	template <typename V>                                                   \
	using P##FrameAllocator = BasePoolAllocator<V, Pool::P, Period::Frame>;

	TYPEDEF_ALLOCATOR(Events);
	TYPEDEF_ALLOCATOR(UI);
	TYPEDEF_ALLOCATOR(Debug);
	TYPEDEF_ALLOCATOR(Generic);

#undef TYPEDEF_ALLOCATOR

	void Init();

	void DebugDisplay();

	constexpr size_t Kilo = 1024;
	constexpr size_t Meg = Kilo * Kilo;
	constexpr size_t Gig = Meg * Kilo;

	template <template <class> typename AllocT, typename T, typename... Vs>
	inline static T* New(Vs&&... args)
	{
		using A = AllocT<T>;
		USING_STANDARD_TYPEDEFS(A);

		A alloc;
		pointer p = alloc.allocate(1);
		::new (typename A::void_pointer(p)) value_type(hrt::forward<Vs>(args)...);

		return p;
	}

	template <template <class> typename AllocT, typename T>
	inline static void Delete(T*& ptr)
	{
		using A = AllocT<T>;
		Free<A::TargetPool, A::TargetPeriod>(ptr);

		ptr = nullptr;
	}

	template <template <class> typename AllocT, typename T>
	struct PoolDeleter
	{
		void operator()(T* ptr) const noexcept
		{
			Delete<AllocT, T>(ptr);
		}
	};

	template <typename T, template <class> typename AllocT>
	using unique_ptr_a = hrt::unique_ptr<T, PoolDeleter<AllocT, T>>;

	template <template <class> typename AllocT, typename T, typename... Vs>
	unique_ptr_a<T, AllocT> MakeUnique(Vs&&... args)
	{
		T* p = New<AllocT, T>(hrt::forward<Vs>(args)...);
		return hrt::unique_ptr<T, PoolDeleter<AllocT, T>>(p);
	}
}
