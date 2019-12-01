#pragma once

#include <heart/canonical_operators.h>
#include <heart/config.h>
#include <heart/copy_move_semantics.h>
#include <heart/types.h>

#include <heart/stl/util/canonical_typedefs.h>

#include <heart/stl/allocator.h>
#include <heart/stl/iterator.h>
#include <heart/stl/utility.h>

// We include initializer_list from STD no matter what
#include <initializer_list>

// we include <string> for memset... bleck
#include <string.h>

#if !HEART_STRICT_PERF
#include <heart/debug/assert.h>
#endif

#if HEART_IS_STD
#include <vector>
#endif

namespace hrt
{
#if HEART_IS_STD
	using namespace std;
#else

	template <typename t, typename alloc = allocator<t>>
	class vector
	{
	public:
		DECLARE_STANDARD_TYPEDEFS(t)

		typedef alloc alloc;
		typedef typename alloc::size_type size_type;
		typedef typename alloc::difference_type difference_type;

		typedef iterator<value_type, size_type, difference_type> iterator;
		typedef const_iterator<value_type, size_type, difference_type> const_iterator;
		typedef hrt::reverse_iterator<iterator> reverse_iterator;
		typedef hrt::reverse_iterator<const_iterator> const_reverse_iterator;

		typedef hrt::allocator_traits<alloc> alloc_traits;
		typedef vector this_type;

	private:
		static constexpr size_type GROWTH_RATE = 2;

		alloc allocator_;
		size_type capacity_ = 0;
		size_type size_ = 0;
		void* data_begin_ = nullptr;

		pointer reallocate(size_type new_capacity)
		{
			size_type original_capacity = capacity_;
			pointer original_data = pointer(data_begin_);
			pointer original_val_end = original_data + size_;

			capacity_ = new_capacity;
			data_begin_ = alloc_traits::allocate(allocator_, capacity_, data_begin_);

			pointer target = pointer(data_begin_);
			pointer new_end = target + capacity_;

			for (auto iter = original_data; iter < original_val_end && target < new_end; ++iter, ++target)
			{
				if constexpr (hrt::is_move_constructible_v<value_type>)
				{
					alloc_traits::construct(allocator_, target, hrt::move(*iter));
				}
				else
				{
					alloc_traits::construct(allocator_, target, *iter);
				}

				alloc_traits::destroy(allocator_, iter);
			}

			alloc_traits::deallocate(allocator_, original_data, original_capacity);
			return pointer(data_begin_) + original_capacity;
		}

		pointer push_back_get_location()
		{
			pointer new_location;

			if (capacity() == 0)
				new_location = reallocate(1);
			else if (size() == capacity())
				new_location = reallocate(GROWTH_RATE * capacity());
			else
				new_location = pointer(data_begin_) + size_;

			++size_;
			return new_location;
		}

		pointer insert_get_location(const_iterator target)
		{
			if (target == end() || size() == 0)
			{
				return push_back_get_location();
			}

			difference_type insert_offset = target - cbegin();

			// We have room to spare
			if (capacity_ - size_ > 1)
			{
				// "allocate" a new element
				pointer insert_target = pointer(data_begin_) + insert_offset;
				pointer old_back = pointer(data_begin_) + size_;
				pointer new_back = push_back_get_location();

				for (pointer src = old_back - 1, tgt = new_back - 1; src >= insert_target; --src, --tgt)
				{
					if constexpr (hrt::is_move_constructible_v<value_type>)
					{
						alloc_traits::construct(allocator_, tgt, hrt::move(*src));
					}
					else
					{
						alloc_traits::construct(allocator_, tgt, *src);
					}

					alloc_traits::destroy(allocator_, src);
				}

				return insert_target;
			}
			else // we have to reallocate, but let's do it ourselves (instead of calling reallocate()) so that things
				 // don't get moved twice
			{
				size_type new_capacity = capacity_ * GROWTH_RATE;
				size_type orig_capacity = capacity_;

				pointer orig_data = pointer(data_begin_);
				pointer orig_end = pointer(data_begin_) + size_;
				pointer orig_insert = pointer(data_begin_) + insert_offset;

				capacity_ = new_capacity;
				data_begin_ = alloc_traits::allocate(allocator_, capacity_, data_begin_);

				pointer new_data = pointer(data_begin_);
				pointer new_cap_end = pointer(data_begin_) + capacity_;

				// Move everything up to our insert goal
				pointer src, tgt;
				for (src = orig_data, tgt = new_data; src < orig_insert; ++src, ++tgt)
				{
					if constexpr (hrt::is_move_constructible_v<value_type>)
					{
						alloc_traits::construct(allocator_, tgt, hrt::move(*src));
					}
					else
					{
						alloc_traits::construct(allocator_, tgt, *src);
					}

					alloc_traits::destroy(allocator_, src);
				}

				// Move everything after our insert goal + 1
				++tgt;
				for (; src < orig_end && tgt < new_cap_end; ++src, ++tgt)
				{
					if constexpr (hrt::is_move_constructible_v<value_type>)
					{
						alloc_traits::construct(allocator_, tgt, hrt::move(*src));
					}
					else
					{
						alloc_traits::construct(allocator_, tgt, *src);
					}

					alloc_traits::destroy(allocator_, src);
				}

				alloc_traits::deallocate(allocator_, orig_data, orig_capacity);

				return new_data + insert_offset;
			}
		}

	public:
		vector() noexcept : capacity_(0), size_(0)
		{
		}

		explicit vector(size_type initial_capacity) : capacity_(0), size_(0), data_begin_(nullptr)
		{
			reserve(initial_capacity);
		}

		explicit vector(size_type initial_size, const_reference initial_value) :
			capacity_(0), size_(0), data_begin_(nullptr)
		{
			resize(initial_size, initial_value);
		}

		vector(const std::initializer_list<value_type>& v) : capacity_(0), size_(0), data_begin_(nullptr)
		{
			reserve(v.size());
			for (auto& val : v)
				emplace_back(val);
		}

#if HEART_STRICT_PERF
		DISABLE_COPY_SEMANTICS(vector);
#else
		vector(const vector& other) : capacity_(0), size_(0), data_begin_(nullptr)
		{
			reserve(other.size());
			for (auto& val : other)
				push_back(val);
		}

		vector& operator=(const vector& other)
		{
			vector tmp(other);
			swap(tmp);
			return *this;
		}
#endif

		vector(vector&& other) : capacity_(0), size_(0), data_begin_(nullptr)
		{
			swap(other);
		}

		vector& operator=(vector&& other)
		{
			swap(other);
		}

		~vector()
		{
			clear();
		}

		// **** ITERATORS **** */
		iterator begin()
		{
			return pointer(data_begin_);
		}

		const_iterator begin() const
		{
			return cbegin();
		}

		const_iterator cbegin() const
		{
			return const_pointer(data_begin_);
		}

		iterator end()
		{
			return pointer(data_begin_) + size_;
		}

		const_iterator end() const
		{
			return cend();
		}

		const_iterator cend() const
		{
			return const_pointer(data_begin_) + size_;
		}

		reverse_iterator rbegin()
		{
			return {pointer(data_begin_) + size_ - 1};
		}

		const_reverse_iterator rbegin() const
		{
			return crbegin();
		}

		const_reverse_iterator crbegin() const
		{
			return pointer(data_begin_) + size_ - 1;
		}

		reverse_iterator rend()
		{
			return {pointer(data_begin_) - 1};
		}

		const_reverse_iterator rend() const
		{
			return crend();
		}

		const_reverse_iterator crend() const
		{
			return pointer(data_begin_) - 1;
		}

		// **** ELEMENT ACCESS **** */
		reference at(size_type index)
		{
#if !HEART_STRICT_PERF
			HEART_CHECK(index < size_);
#endif
			return *(pointer(data_begin_) + index);
		}

		const_reference at(size_type index) const
		{
#if !HEART_STRICT_PERF
			HEART_CHECK(index < size_);
#endif
			return *(pointer(data_begin_) + index);
		}

		reference operator[](size_type index)
		{
			return at(index);
		}

		const_reference operator[](size_type index) const
		{
			return at(index);
		}

		reference front()
		{
			return at(0);
		}

		const_reference front() const
		{
			return at(0);
		}

		reference back()
		{
			return at(size_ - 1);
		}

		const_reference back() const
		{
			return at(size_ - 1);
		}

		pointer data()
		{
			return pointer(data_begin_);
		}

		// **** CAPACITY **** */
		inline size_type size() const
		{
			return size_;
		}

		inline size_type max_size() const
		{
			return std::numeric_limits<size_type>::max();
		}

		inline size_type capacity() const
		{
			return capacity_;
		}

		inline bool empty() const
		{
			return size() == 0;
		}

		const_pointer data() const
		{
			return const_pointer(data_begin_);
		}

		void resize(size_type count)
		{
			resize(count, value_type());
		}

		void resize(size_type count, const_reference val)
		{
			if (count > size_)
			{
				reallocate(count);

				if constexpr (hrt::is_trivially_copyable_v<value_type>)
				{
					size_ = count;
					if constexpr (sizeof(value_type) <= sizeof(uint8_t))
					{
						std::memset(data_begin_, uint8_t(val), size_);
					}
					else
					{
						for (size_t i = 0; i < size_; ++i)
							*(pointer(data_begin_) + i) = val;
					}
				}
				else
				{
					while (size_ < capacity_)
						push_back(val);
				}
			}
			else if (count < size_)
			{
				reserve(count);
			}
		}

		void reserve(size_type count)
		{
			if (count > size_)
			{
				reallocate(count);
			}
			else if (count < size_)
			{
				pointer data = pointer(data_begin_);
				for (pointer iter = data + size_ - 1; iter != data + count; --iter)
				{
					alloc_traits::destroy(allocator_, iter);
					--size_;
				}

				reallocate(count);
			}
		}

		// **** MODIFIERS **** */
		void push_back(const_reference other)
		{
			emplace_back(other);
		}

		void push_back(value_type&& other)
		{
			emplace_back(hrt::move(other));
		}

		void pop_back()
		{
			pointer end = pointer(data_begin_) + --size_;
			alloc_traits::destroy(allocator_, end);

#if !HEART_STRICT_PERF
			memset(end, 0, sizeof(value_type));
#endif
		}

		void insert(const_iterator position, const_reference val)
		{
			pointer loc = insert_get_location(position);
			alloc_traits::construct(allocator_, loc, val);
		}

		void insert(const_iterator position, value_type&& val)
		{
			pointer loc = insert_get_location(position);
			alloc_traits::construct(allocator_, loc, hrt::forward<value_type>(val));
		}

		void erase(iterator position)
		{
			erase(position, position + 1);
		}

		void erase(iterator first, iterator last)
		{
			size_type destroyed_count = 0;
			pointer destruct_ptr = &*first;
			pointer destruct_end = &*last;

			for (; destruct_ptr < destruct_end; ++destruct_ptr, ++destroyed_count)
			{
				alloc_traits::destroy(allocator_, destruct_ptr);
			}

			pointer end = pointer(data_begin_) + size_;
			for (; destruct_ptr < end; ++destruct_ptr)
			{
				pointer tgt = destruct_ptr - destroyed_count;
				alloc_traits::construct(allocator_, tgt, hrt::move(*destruct_ptr));
				alloc_traits::destroy(allocator_, destruct_ptr);
			}

			size_ -= destroyed_count;
		}

		void swap(this_type& other) noexcept
		{
			hrt::swap(this->allocator_, other.allocator_);
			hrt::swap(this->size_, other.size_);
			hrt::swap(this->capacity_, other.capacity_);
			hrt::swap(this->data_begin_, other.data_begin_);
		}

		void clear()
		{
			pointer begin = pointer(data_begin_);
			pointer end = begin + size_;

			while (begin < end)
			{
				alloc_traits::destroy(allocator_, begin++);
			}

			alloc_traits::deallocate(allocator_, pointer(data_begin_), capacity_);
			data_begin_ = nullptr;
			capacity_ = 0;
			size_ = 0;
		}

		template <typename... Ts>
		reference emplace_back(Ts&&... args)
		{
			pointer loc = push_back_get_location();
			alloc_traits::construct(allocator_, loc, hrt::forward<Ts>(args)...);
			return *loc;
		}

		template <typename... Ts>
		iterator emplace(const_iterator position, Ts&&... args)
		{
			pointer loc = insert_get_location(position);
			alloc_traits::construct(allocator_, loc, hrt::forward<Ts>(args)...);
			return iterator(loc);
		}
	};
#endif
}
