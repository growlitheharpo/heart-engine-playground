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

#include <heart/allocator.h>

#include <atomic>
#include <mutex>
#include <unordered_map>

#include <gtest/gtest.h>

struct TestTrackingAllocator final : public HeartBaseAllocator
{
	using size_type = HeartBaseAllocator::size_type;
	using difference_type = HeartBaseAllocator::difference_type;
	using void_pointer = HeartBaseAllocator::void_pointer;

	std::atomic_uint64_t m_allocatedSize;
	std::atomic_uint64_t m_allocatedCount;
	std::mutex m_mapMutex;
	std::unordered_map<void*, size_t> m_allocationMap;

	void* RawAllocate(size_type n, void* hint = nullptr) override
	{
		void* ptr = operator new(n);

		++m_allocatedCount;
		m_allocatedSize += n;

		{
			std::lock_guard lock(m_mapMutex);
			m_allocationMap.emplace(std::make_pair(ptr, n));
		}

		return ptr;
	}

	void RawDeallocate(void* p, size_type n = 1) override
	{
		--m_allocatedCount;

		{
			std::lock_guard lock(m_mapMutex);

			auto iter = m_allocationMap.find(p);
			EXPECT_TRUE(iter != m_allocationMap.end());

			if (iter != m_allocationMap.end())
			{
				m_allocatedSize -= iter->second;
				m_allocationMap.erase(iter);
			}
		}

		operator delete(p);
	}
};
