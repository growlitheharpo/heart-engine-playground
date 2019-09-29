#pragma once

#include <heart/config.h>
#include <heart/stl/type_traits.h>

#define USE_STD_ALLOCATOR 0

#if HEART_IS_STD || USE_STD_ALLOCATOR
#include <memory>
#endif

namespace hrt
{
#if HEART_IS_STD || USE_STD_ALLOCATOR
	using namespace std;
#else

	// In C++20, the standard allocator is totally stateless (and basically useless)
	template <typename T>
	struct allocator
	{
		typedef T value_type;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		using propogate_on_container_move_assignment = hrt::true_type;
		using is_always_equal = hrt::true_type;

		value_type* allocate(size_type n)
		{
			void* ptr = operator new(n * sizeof(value_type));
			return reinterpret_cast<value_type*>(ptr);
		}

		void deallocate(value_type* p, size_type n)
		{
			operator delete(p, n * sizeof(value_type));
		}
	};

	template <typename alloc_t>
	struct allocator_traits
	{
		using allocator_type = alloc_t;
		using value_type = typename allocator_type::value_type;
		using pointer = typename allocator_type::value_type*;
		using const_pointer = const typename allocator_type::value_type*;
		using void_pointer = void*;
		using const_void_pointer = const void*;

		using size_type = typename allocator_type::size_type;
		using difference_type = typename allocator_type::difference_type;

		static pointer allocate(allocator_type& a, size_type n)
		{
			return a.allocate(n);
		}

		static pointer allocate(allocator_type& a, size_type n, const_void_pointer hint)
		{
			return a.allocate(n);
		}

		static void deallocate(allocator_type& a, pointer p, size_type n)
		{
			a.deallocate(p, n);
		}

		template <typename Ptr, typename... Args>
		static void construct(allocator_type&, const Ptr p, Args&&... args)
		{
			::new (void_pointer(p)) value_type(hrt::forward<Args>(args)...);
		}

		template <typename Ptr>
		static void destroy(allocator_type&, const Ptr p)
		{
			pointer(p)->~value_type();
		}
	};
#endif
}
