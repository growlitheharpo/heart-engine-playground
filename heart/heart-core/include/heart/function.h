#pragma once

#include <heart/copy_move_semantics.h>

#include <heart/stl/forward.h>
#include <heart/stl/move.h>
#include <heart/stl/type_traits/aligned_storage.h>
#include <heart/stl/type_traits/enable_if.h>

#include <heart/debug/assert.h>

namespace details
{
	template <typename R, typename... Args>
	class HeartFunctionBase
	{
	public:
		virtual R Call(Args&&... args) = 0;
		virtual void Move(void* location) = 0;
		virtual ~HeartFunctionBase() = default;
	};

	template <typename F, typename R, typename... Args>
	class HeartFunctionImpl : public HeartFunctionBase<R, Args...>
	{
		F f;

	public:
		HeartFunctionImpl(F&& f) :
			f(hrt::forward<F>(f))
		{
		}

		~HeartFunctionImpl() = default;

		R Call(Args&&... args) override
		{
			return f(hrt::forward<Args>(args)...);
		}

		void Move(void* location) override
		{
			new (location) HeartFunctionImpl<F, R, Args...>(hrt::move(f));
		}
	};
}

template <typename Sig, size_t Storage = 32>
class HeartFunction;

template <typename R, typename... Args, size_t Storage>
class HeartFunction<R(Args...), Storage>
{
private:
	using BaseType = details::HeartFunctionBase<R, Args...>;

	using StorageType = hrt::aligned_storage_t<Storage>;

	template <typename F>
	using ImplType = details::HeartFunctionImpl<F, R, Args...>;

	bool m_set = false;
	StorageType m_storage;

	BaseType* GetPtr()
	{
		return reinterpret_cast<BaseType*>(&m_storage);
	}

	template <typename F>
	static constexpr bool SizeCheck = (sizeof(ImplType<F>) <= sizeof(m_storage));

public:
	HeartFunction() = default;

	~HeartFunction()
	{
		Clear();
	}

	DISABLE_COPY_SEMANTICS(HeartFunction);

	HeartFunction(HeartFunction&& o) :
		HeartFunction()
	{
		Swap(o);
	}

	HeartFunction& operator=(HeartFunction&& o)
	{
		Swap(o);
		return *this;
	}

	template <typename F, hrt::enable_if_t<SizeCheck<F>, void*> = nullptr>
	HeartFunction(F&& f)
	{
		Set(hrt::forward<F>(f));
	}

	void Swap(HeartFunction& other)
	{
		if (m_set != other.m_set)
		{
			HeartFunction* wasSet;
			HeartFunction* toSet;

			if (m_set)
			{
				wasSet = this;
				toSet = &other;
			}
			else
			{
				wasSet = &other;
				toSet = this;
			}

			wasSet->GetPtr()->Move(toSet->GetPtr());
			wasSet->Clear();
			toSet->m_set = true;
		}
		else
		{
			HeartFunction tmp;

			// Put "other" in tmp
			tmp.Swap(other);

			// Put this in other
			other.Swap(*this);

			// Put "other" in this (via tmp)
			tmp.Swap(*this);
		}
	}

	template <typename F, hrt::enable_if_t<SizeCheck<F>, void*> = nullptr>
	void Set(F&& f)
	{
		new (GetPtr()) ImplType<F>(hrt::forward<F>(f));
		m_set = true;
	}

	void Clear()
	{
		GetPtr()->~BaseType();
		m_set = false;
	}

	explicit operator bool() const
	{
		return m_set;
	}

	R operator()(Args... args)
	{
		HEART_ASSERT(m_set);
		return GetPtr()->Call(hrt::forward<Args>(args)...);
	}
};
