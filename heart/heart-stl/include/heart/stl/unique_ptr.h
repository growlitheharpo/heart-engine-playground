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

#include <heart/stl/forward.h>
#include <heart/stl/move.h>
#include <heart/stl/type_traits.h>
#include <heart/stl/utility.h>

namespace hrt
{
	template <typename T>
	struct default_delete
	{
		constexpr default_delete() noexcept = default;

		template <typename U, enable_if_t<is_convertible_v<T*, U*>, int> = 0>
		default_delete(const default_delete<U>&)
		{
		}

		void operator()(T* ptr) const noexcept
		{
			static_assert(0 < sizeof(T), "Cannot delete an incomplete type");
			delete ptr;
		}
	};

	template <typename T>
	struct default_delete<T[]>
	{
		constexpr default_delete() noexcept = default;

		template <typename U, enable_if_t<is_convertible_v<U (*)[], T (*)[]>, int> = 0>
		default_delete(const default_delete<U[]>&)
		{
		}

		template <class U, enable_if_t<is_convertible_v<U (*)[], T (*)[]>, int> = 0>
		void operator()(U* ptr) const noexcept
		{
			static_assert(0 < sizeof(U), "Cannot delete an incomplete type");
			delete[] ptr;
		}
	};

	template <typename T, typename D = default_delete<T>>
	class unique_ptr
	{
	public:
		using pointer = T*;
		using element_type = T;
		using deleter_type = D;
		using nullptr_t = decltype(nullptr);

		// TODO: audit pretty much all uses of D, especially with a stateful deleter.
		// I think everything here is incorrect if D is stateful.

	private:
		D m_deleter;
		pointer m_pointer = nullptr;

	public:
		template <enable_if_t<!is_pointer_v<D> && is_default_constructible_v<D>, int> = 0>
		constexpr unique_ptr() noexcept :
			m_deleter(),
			m_pointer(nullptr)
		{
		}

		template <enable_if_t<!is_pointer_v<D> && is_default_constructible_v<D>, int> = 0>
		constexpr unique_ptr(nullptr_t) noexcept :
			m_deleter(),
			m_pointer(nullptr)
		{
		}

		template <enable_if_t<!is_pointer_v<D> && is_default_constructible_v<D>, int> = 0>
		explicit unique_ptr(pointer p) noexcept :
			m_deleter(),
			m_pointer(p)
		{
		}

		template <typename AltD, enable_if_t<is_constructible_v<D, AltD>, int> = 0>
		unique_ptr(pointer p, const AltD& d) noexcept :
			m_deleter(d),
			m_pointer(p)
		{
		}

		template <enable_if_t<is_move_constructible_v<D>, int> = 0>
		unique_ptr(unique_ptr&& u) noexcept :
			m_deleter(hrt::forward(u.m_deleter)),
			m_pointer(nullptr)
		{
			reset(u.release());
		}

		unique_ptr& operator=(unique_ptr&& u) noexcept
		{
			if (this != addressof(u))
			{
				reset(u.release());
				m_deleter = hrt::move(u.m_deleter);
			}

			return *this;
		}

		template <typename U, typename E, enable_if_t<is_convertible_v<typename unique_ptr<U, E>::pointer, pointer>, int> = 0>
		unique_ptr(unique_ptr<U, E>&& u) noexcept :
			m_deleter(u.m_deleter),
			m_pointer(u.m_pointer)
		{
			u.m_pointer = nullptr;
		}

		// TODO: move assignment from unique_ptr<U, E>

		unique_ptr(const unique_ptr&) = delete;

		unique_ptr& operator=(const unique_ptr&) = delete;

		~unique_ptr()
		{
			reset();
		}

		add_lvalue_reference<T> operator*() const
		{
			return *m_pointer;
		}

		pointer operator->() const noexcept
		{
			return m_pointer;
		}

		pointer get() const noexcept
		{
			return m_pointer;
		}

		deleter_type& get_deleter() noexcept
		{
			return m_deleter;
		}

		const deleter_type& get_deleter() const noexcept
		{
			return m_deleter;
		}

		explicit operator bool() const noexcept
		{
			return m_pointer != nullptr;
		}

		pointer release() noexcept
		{
			pointer v = m_pointer;
			m_pointer = nullptr;
			return v;
		}

		void reset(pointer p = pointer()) noexcept
		{
			pointer old_p = m_pointer;
			m_pointer = p;

			if (old_p != nullptr)
			{
				m_deleter(old_p);
			}
		}

		void swap(unique_ptr& u) noexcept
		{
			std::swap(u.m_pointer, m_pointer);
			std::swap(u.m_deleter, m_deleter);
		}
	};

	// TODO: unique_ptr<T[]>
	// but also, just use a vector.

	template <typename T, typename... Args>
	unique_ptr<T> make_unique(Args&&... args)
	{
		return unique_ptr<T>(new T(hrt::forward<Args>(args)...));
	}
}
