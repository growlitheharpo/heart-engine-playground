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

#include <heart/copy_move_semantics.h>
#include <heart/util/tag_type.h>

#include <heart/debug/assert.h>

#include <heart/stl/forward.h>

class HeartMutex
{
private:
	void* m_handle;

public:
	HeartMutex();
	~HeartMutex();

	DISABLE_COPY_AND_MOVE_SEMANTICS(HeartMutex);

	void LockExclusive();
	bool TryLockExclusive();

	void LockShared();
	bool TryLockShared();

	void Unlock();
	void UnlockShared();

	void** NativeHandle()
	{
		return &m_handle;
	}
};

struct HeartLockMethod
{
	HEART_DECLARE_TAG_TYPE(Defer);

	HEART_DECLARE_TAG_TYPE(Adopt);

	HEART_DECLARE_TAG_TYPE(TryLock);
};

template <typename MutexT>
class HeartUniqueLock
{
private:
	MutexT* m_mutex;
	bool m_owns = false;

public:
	template <typename... LockArgs>
	HeartUniqueLock(MutexT& mutex, LockArgs... args) :
		m_mutex(&mutex), m_owns(false)
	{
		Lock(hrt::forward<LockArgs>(args)...);
	}

	HeartUniqueLock(MutexT& mutex, HeartLockMethod::DeferT) noexcept :
		m_mutex(&mutex), m_owns(false)
	{
	}

	template <typename... LockArgs>
	HeartUniqueLock(MutexT& mutex, HeartLockMethod::TryLockT, LockArgs... args) :
		m_mutex(&mutex), m_owns(false)
	{
		TryLock(hrt::forward<LockArgs>(args)...);
	}

	HeartUniqueLock(MutexT& mutex, HeartLockMethod::AdoptT) noexcept :
		m_mutex(&mutex), m_owns(true)
	{
	}

	DISABLE_COPY_SEMANTICS(HeartUniqueLock);

	HeartUniqueLock(HeartUniqueLock&& o) :
		m_mutex(nullptr), m_owns(false)
	{
		Swap(o);
	}

	HeartUniqueLock& operator=(HeartUniqueLock&& o)
	{
		if (this != &o)
		{
			if (m_owns)
			{
				Unlock();
			}

			m_mutex = o.m_mutex;
			m_owns = o.m_owns;
			o.m_mutex = nullptr;
			o.m_owns = false;
		}

		return *this;
	}

	~HeartUniqueLock()
	{
		if (m_owns)
		{
			Unlock();
		}
	}

	template <typename... LockArgs>
	void Lock(LockArgs... args)
	{
		HEART_ASSERT(m_mutex != nullptr);
		HEART_ASSERT(!m_owns);

		m_mutex->LockExclusive(hrt::forward<LockArgs>(args)...);
		m_owns = true;
	}

	template <typename... LockArgs>
	bool TryLock(LockArgs... args)
	{
		HEART_ASSERT(m_mutex != nullptr);
		HEART_ASSERT(!m_owns);

		m_owns = m_mutex->TryLockExclusive(hrt::forward<LockArgs>(args)...);
		return m_owns;
	}

	void Unlock()
	{
		HEART_ASSERT(m_mutex != nullptr);
		HEART_ASSERT(m_owns);

		m_mutex->Unlock();
		m_owns = false;
	}

	bool OwnsLock() const
	{
		return m_owns;
	}

	HeartUniqueLock& Swap(HeartUniqueLock& o)
	{
		if (this != &o)
		{
			auto p = m_mutex;
			m_mutex = o.m_mutex;
			o.m_mutex = p;

			auto b = m_owns;
			m_owns = o.m_owns;
			o.m_owns = m_owns;
		}

		return *this;
	}
};

template <typename MutexT>
class HeartSharedLock
{
private:
	MutexT* m_mutex;
	bool m_owns = false;

public:
	HeartSharedLock(MutexT& mutex) :
		m_mutex(&mutex), m_owns(false)
	{
		Lock();
	}

	HeartSharedLock(MutexT& mutex, HeartLockMethod::DeferT) noexcept :
		m_mutex(&mutex), m_owns(false)
	{
	}

	HeartSharedLock(MutexT& mutex, HeartLockMethod::TryLockT) :
		m_mutex(&mutex), m_owns(false)
	{
		TryLock();
	}

	HeartSharedLock(MutexT& mutex, HeartLockMethod::AdoptT) noexcept :
		m_mutex(&mutex), m_owns(true)
	{
	}

	DISABLE_COPY_SEMANTICS(HeartSharedLock);

	HeartSharedLock(HeartSharedLock&& o) :
		m_mutex(nullptr), m_owns(false)
	{
		Swap(o);
	}

	HeartSharedLock& operator=(HeartSharedLock&& o)
	{
		if (this != &o)
		{
			if (m_owns)
			{
				Unlock();
			}

			m_mutex = o.m_mutex;
			m_owns = o.m_owns;
			o.m_mutex = nullptr;
			o.m_owns = false;
		}

		return *this;
	}

	~HeartSharedLock()
	{
		if (m_owns)
		{
			Unlock();
		}
	}

	void Lock()
	{
		HEART_ASSERT(m_mutex != nullptr);
		HEART_ASSERT(!m_owns);

		m_mutex->LockShared();
		m_owns = true;
	}

	bool TryLock()
	{
		HEART_ASSERT(m_mutex != nullptr);
		HEART_ASSERT(!m_owns);

		m_owns = m_mutex->TryLockShared();
		return m_owns;
	}

	void Unlock()
	{
		HEART_ASSERT(m_mutex != nullptr);
		HEART_ASSERT(m_owns);

		m_mutex->UnlockShared();
		m_owns = false;
	}

	bool OwnsLock() const
	{
		return m_owns;
	}

	HeartSharedLock& Swap(HeartSharedLock& o)
	{
		if (this != &o)
		{
			auto p = m_mutex;
			m_mutex = o.m_mutex;
			o.m_mutex = p;

			auto b = m_owns;
			m_owns = o.m_owns;
			o.m_owns = m_owns;
		}

		return *this;
	}
};

template <typename MutexT>
class HeartLockGuard
{
private:
	HeartUniqueLock<MutexT> m_innerLock;

public:
	template <typename... LockArgs>
	HeartLockGuard(MutexT& m, LockArgs... args) :
		m_innerLock(m, hrt::forward<LockArgs>(args)...)
	{
		HEART_ASSERT(m_innerLock.OwnsLock());
	}
};

template <typename MutexT>
class HeartSharedLockGuard
{
private:
	HeartSharedLock<MutexT> m_innerLock;

public:
	HeartSharedLockGuard(MutexT& m) :
		m_innerLock(m)
	{
		HEART_ASSERT(m_innerLock.OwnsLock());
	}
};
