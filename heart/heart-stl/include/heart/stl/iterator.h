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
#include <heart/stl/util/canonical_typedefs.h>
#include <heart/types.h>

#include <heart/canonical_operators.h>
#include <heart/copy_move_semantics.h>

namespace hrt
{
	using size_t = size_t;

	template <typename DataType, typename s = size_t, typename d = ptrdiff_t>
	class iterator;

	template <typename DataType, typename s = size_t, typename d = ptrdiff_t>
	class const_iterator;

	template <typename IteratorType>
	class reverse_iterator;

	struct input_iterator_tag
	{
	};

	struct output_iterator_tag
	{
	};

	struct forward_iterator_tag : input_iterator_tag
	{
	};

	struct bidirectional_iterator_tag : forward_iterator_tag
	{
	};

	struct random_access_iterator_tag : bidirectional_iterator_tag
	{
	};

	template <typename DataType, typename s, typename d>
	class iterator
	{
	public:
		DECLARE_STANDARD_TYPEDEFS(DataType);

		typedef random_access_iterator_tag iterator_category;
		typedef s size_type;
		typedef d difference_type;
		typedef iterator<DataType, size_type, difference_type> this_type;

		template <typename DataType, typename s, typename d>
		friend class const_iterator;

	private:
		pointer data_ptr_;

	public:
		iterator() :
			data_ptr_(nullptr)
		{
		}

		iterator(pointer l) :
			data_ptr_(l)
		{
		}

		USE_DEFAULT_COPY_SEMANTICS(iterator);
		USE_DEFAULT_MOVE_SEMANTICS(iterator);

		~iterator() = default;

		inline this_type& operator+=(difference_type offset)
		{
			data_ptr_ += offset;
			return *this;
		}

		inline this_type& operator-=(difference_type offset)
		{
			data_ptr_ -= offset;
			return *this;
		}

#if HEART_STRICT_PERF
		DECLARE_CANONICAL_ADDITION_OPERATORS_NOPOST(this_type, difference_type)
		DECLARE_CANONICAL_SUBTRACTION_OPERATORS_NOPOST(this_type, difference_type)
#else
		DECLARE_CANONICAL_ADDITION_OPERATORS(this_type, difference_type)
		DECLARE_CANONICAL_SUBTRACTION_OPERATORS(this_type, difference_type)
#endif

		difference_type operator-(const this_type& other)
		{
			return data_ptr_ - other.data_ptr_;
		}

		REF_AND_CONST_REF(operator*(), { return *data_ptr_; });
		REF_AND_CONST_REF(operator[](difference_type offset), { return operator+(offset).operator*(); });

		bool operator==(const this_type& o) const
		{
			return data_ptr_ == o.data_ptr_;
		}

		bool operator<(const this_type& o) const
		{
			return data_ptr_ < o.data_ptr_;
		}

		DECLARE_CANONICAL_COMPARISON_OPERATORS(this_type);
	};

	template <typename DataType, typename s, typename d>
	class const_iterator
	{
	public:
		DECLARE_STANDARD_TYPEDEFS(DataType);

		typedef random_access_iterator_tag iterator_category;
		typedef s size_type;
		typedef d difference_type;
		typedef const_iterator<DataType, size_type, difference_type> this_type;
		typedef iterator<DataType, s, d> base_iterator;

	private:
		const_pointer data_ptr_;

	public:
		const_iterator() :
			data_ptr_(nullptr)
		{
		}

		const_iterator(const_pointer l) :
			data_ptr_(l)
		{
		}

		USE_DEFAULT_COPY_SEMANTICS(const_iterator);
		USE_DEFAULT_MOVE_SEMANTICS(const_iterator);

		const_iterator(const base_iterator& o) noexcept :
			data_ptr_(o.data_ptr_)
		{
		}

		const_iterator(base_iterator&& o) noexcept :
			data_ptr_(o.data_ptr_)
		{
		}

		~const_iterator() = default;

		inline this_type& operator+=(difference_type offset)
		{
			data_ptr_ += offset;
			return *this;
		}

		inline this_type& operator-=(difference_type offset)
		{
			data_ptr_ -= offset;
			return *this;
		}

#if HEART_STRICT_PERF
		DECLARE_CANONICAL_ADDITION_OPERATORS_NOPOST(this_type, difference_type)
		DECLARE_CANONICAL_SUBTRACTION_OPERATORS_NOPOST(this_type, difference_type)
#else
		DECLARE_CANONICAL_ADDITION_OPERATORS(this_type, difference_type)
		DECLARE_CANONICAL_SUBTRACTION_OPERATORS(this_type, difference_type)
#endif

		difference_type operator-(const this_type& other)
		{
			return data_ptr_ - other.data_ptr_;
		}

		const_reference operator*() const
		{
			return *data_ptr_;
		};

		const_reference operator[](difference_type offset) const
		{
			return operator+(offset).operator*();
		};

		bool operator==(const this_type& o) const
		{
			return data_ptr_ == o.data_ptr_;
		}

		bool operator<(const this_type& o) const
		{
			return data_ptr_ < o.data_ptr_;
		}

		DECLARE_CANONICAL_COMPARISON_OPERATORS(this_type);

		bool operator==(const base_iterator& o) const
		{
			return data_ptr_ == o.data_ptr_;
		}

		bool operator<(const base_iterator& o) const
		{
			return data_ptr_ < o.data_ptr_;
		}

		DECLARE_CANONICAL_COMPARISON_OPERATORS(base_iterator);
	};

	template <typename IteratorType>
	class reverse_iterator
	{
	public:
		typedef IteratorType iterator;
		USING_STANDARD_TYPEDEFS(iterator);

		using difference_type = typename iterator::difference_type;
		typedef reverse_iterator<IteratorType> this_type;

	private:
		iterator data_ptr_;

	public:
		reverse_iterator() :
			data_ptr_(nullptr)
		{
		}

		reverse_iterator(pointer d) :
			data_ptr_(d)
		{
		}

		USE_DEFAULT_COPY_SEMANTICS(reverse_iterator);
		USE_DEFAULT_MOVE_SEMANTICS(reverse_iterator);

		~reverse_iterator() = default;

		// The "trick" of reverse iterators is that their + operator calls the internal - operator, and vice-versa
		inline this_type& operator+=(difference_type offset)
		{
			data_ptr_ -= offset;
			return *this;
		}

		inline this_type& operator-=(difference_type offset)
		{
			data_ptr_ += offset;
			return *this;
		}

#if HEART_STRICT_PERF
		DECLARE_CANONICAL_ADDITION_OPERATORS_NOPOST(this_type, difference_type)
		DECLARE_CANONICAL_SUBTRACTION_OPERATORS_NOPOST(this_type, difference_type)
#else
		DECLARE_CANONICAL_ADDITION_OPERATORS(this_type, difference_type)
		DECLARE_CANONICAL_SUBTRACTION_OPERATORS(this_type, difference_type)
#endif

		bool operator==(this_type other) const
		{
			return base() == other.base();
		}

		bool operator<(this_type other) const
		{
			return other.base() < base();
		}

		DECLARE_CANONICAL_COMPARISON_OPERATORS_BY_VALUE(this_type);

		difference_type operator-(const this_type& other)
		{
			return data_ptr_ - other.data_ptr_;
		}

		REF_AND_CONST_REF(operator*(), { return *data_ptr_; })
		REF_AND_CONST_REF(operator[](difference_type offset), { return operator+(offset).operator*(); })

		iterator base() const
		{
			return data_ptr_;
		}
	};
}
