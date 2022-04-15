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
#include <heart/memory/intrusive_ptr.h>

#include <gtest/gtest.h>

#include <type_traits>

struct RefCountedType
{
	uint32_t refCount = 0;
};

void HeartIncrementRef(RefCountedType* p)
{
	++p->refCount;
}

void HeartDecrementRef(RefCountedType* p)
{
	--p->refCount;
}

uint32_t HeartGetRefCount(RefCountedType* p)
{
	return p->refCount;
}

TEST(HeartIntrusivePtr, OverloadedRefCounting)
{
	RefCountedType refCounter;
	EXPECT_EQ(refCounter.refCount, 0);

	HeartIntrusivePtr<RefCountedType> p(&refCounter);
	EXPECT_EQ(refCounter.refCount, 1);
	EXPECT_EQ(&refCounter, p.Get());
	EXPECT_EQ(p->refCount, refCounter.refCount);
	EXPECT_EQ((*p).refCount, refCounter.refCount);
	EXPECT_EQ(p.UseCount(), refCounter.refCount);
	EXPECT_TRUE(p);
	EXPECT_TRUE(p != nullptr);
	EXPECT_TRUE(nullptr != p);
	EXPECT_FALSE(!p);
	EXPECT_FALSE(p == nullptr);
	EXPECT_FALSE(nullptr == p);

	p = nullptr;
	EXPECT_EQ(refCounter.refCount, 0);
	EXPECT_TRUE(!p);
	EXPECT_TRUE(p == nullptr);
	EXPECT_TRUE(nullptr == p);
	EXPECT_FALSE(p);
	EXPECT_FALSE(p != nullptr);
	EXPECT_FALSE(nullptr != p);

	p = &refCounter;
	EXPECT_EQ(refCounter.refCount, 1);
}

TEST(HeartIntrusivePtr, Reset)
{
	RefCountedType refCounter;
	EXPECT_EQ(refCounter.refCount, 0);

	HeartIntrusivePtr<RefCountedType> p(&refCounter);
	EXPECT_EQ(refCounter.refCount, 1);

	p.Reset();
	EXPECT_EQ(refCounter.refCount, 0);
}

TEST(HeartIntrusivePtr, Swap)
{
	RefCountedType refCounter;
	EXPECT_EQ(refCounter.refCount, 0);

	HeartIntrusivePtr<RefCountedType> p(&refCounter);
	EXPECT_EQ(refCounter.refCount, 1);

	HeartIntrusivePtr<RefCountedType> q;

	p.Swap(q);
	EXPECT_EQ(refCounter.refCount, 1);
	EXPECT_FALSE(p);
	EXPECT_TRUE(q);
}

TEST(HeartIntrusivePtr, CopyConstructor)
{
	RefCountedType refCounter;
	EXPECT_EQ(refCounter.refCount, 0);

	HeartIntrusivePtr<RefCountedType> p(&refCounter);
	EXPECT_EQ(refCounter.refCount, 1);

	HeartIntrusivePtr<RefCountedType> q(p);
	EXPECT_EQ(refCounter.refCount, 2);
	EXPECT_TRUE(p);
	EXPECT_TRUE(q);
}

TEST(HeartIntrusivePtr, CopyAssignment)
{
	RefCountedType refCounter;
	EXPECT_EQ(refCounter.refCount, 0);

	HeartIntrusivePtr<RefCountedType> p(&refCounter);
	EXPECT_EQ(refCounter.refCount, 1);

	HeartIntrusivePtr<RefCountedType> q = p;
	EXPECT_EQ(refCounter.refCount, 2);
	EXPECT_TRUE(p);
	EXPECT_TRUE(q);
}

TEST(HeartIntrusivePtr, MoveConstructor)
{
	RefCountedType refCounter;
	EXPECT_EQ(refCounter.refCount, 0);

	HeartIntrusivePtr<RefCountedType> p(&refCounter);
	EXPECT_EQ(refCounter.refCount, 1);

	HeartIntrusivePtr<RefCountedType> q(std::move(p));
	EXPECT_EQ(refCounter.refCount, 1);
	EXPECT_FALSE(p);
	EXPECT_TRUE(q);
}

TEST(HeartIntrusivePtr, MoveAssignment)
{
	RefCountedType refCounter1;
	EXPECT_EQ(refCounter1.refCount, 0);

	RefCountedType refCounter2;
	EXPECT_EQ(refCounter2.refCount, 0);

	HeartIntrusivePtr<RefCountedType> p(&refCounter1);
	EXPECT_EQ(refCounter1.refCount, 1);

	HeartIntrusivePtr<RefCountedType> q(&refCounter2);
	EXPECT_EQ(refCounter2.refCount, 1);

	q = std::move(p);
	EXPECT_EQ(refCounter2.refCount, 0);
	EXPECT_EQ(refCounter1.refCount, 1);
	EXPECT_FALSE(p);
	EXPECT_TRUE(q);
}

struct ComplexRefCountedType
{
	uint32_t refCount = 0;

	void IncrementRef()
	{
		++refCount;
	}

	void DecrementRef()
	{
		--refCount;
	}

	uint32_t GetRefCount() const
	{
		return refCount;
	}
};

TEST(HeartIntrusivePtr, TemplateRefCounting)
{
	ComplexRefCountedType refCounter;
	EXPECT_EQ(refCounter.refCount, 0);

	HeartIntrusivePtr<ComplexRefCountedType> p(&refCounter);
	EXPECT_EQ(refCounter.refCount, 1);
	EXPECT_EQ(&refCounter, p.Get());
	EXPECT_EQ(p->refCount, refCounter.refCount);
	EXPECT_EQ((*p).refCount, refCounter.refCount);
	EXPECT_EQ(p.UseCount(), refCounter.refCount);
	EXPECT_TRUE(p);
	EXPECT_TRUE(p != nullptr);
	EXPECT_TRUE(nullptr != p);
	EXPECT_FALSE(!p);
	EXPECT_FALSE(p == nullptr);
	EXPECT_FALSE(nullptr == p);

	p = nullptr;
	EXPECT_EQ(refCounter.refCount, 0);
	EXPECT_TRUE(!p);
	EXPECT_TRUE(p == nullptr);
	EXPECT_TRUE(nullptr == p);
	EXPECT_FALSE(p);
	EXPECT_FALSE(p != nullptr);
	EXPECT_FALSE(nullptr != p);

	p = &refCounter;
	EXPECT_EQ(refCounter.refCount, 1);
}

static bool MemoryManagedTypeDestroyed = false;

struct MemoryManagedType
{
	~MemoryManagedType()
	{
		MemoryManagedTypeDestroyed = true;
	}

	uint32_t refCount = 0;
};

void HeartIncrementRef(MemoryManagedType* p)
{
	++p->refCount;
}

void HeartDecrementRef(MemoryManagedType* p)
{
	if (--p->refCount == 0)
	{
		delete p;
	}
}

uint32_t HeartGetRefCount(MemoryManagedType* p)
{
	return p->refCount;
}

TEST(HeartIntrusivePtr, RealMemoryManagement)
{
	HeartIntrusivePtr<MemoryManagedType> p = new MemoryManagedType();
	EXPECT_TRUE(p);
	EXPECT_EQ(p->refCount, 1);
	EXPECT_EQ(p.UseCount(), 1);
	EXPECT_FALSE(MemoryManagedTypeDestroyed);

	p = nullptr;
	EXPECT_FALSE(p);
	EXPECT_EQ(p.Get(), nullptr);
	EXPECT_EQ(p.UseCount(), 0);
	EXPECT_TRUE(MemoryManagedTypeDestroyed);
}