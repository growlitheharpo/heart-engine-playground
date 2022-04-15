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
#include <heart/function/dynamic_function.h>

#include <gtest/gtest.h>

#include "utils/tracking_allocator.h"

#include <array>

constexpr int SecretValue = 0xF00DFEED;

int DynamicFunctionTestVisibleFunction()
{
	return SecretValue;
}

static int DynamicFunctionTestStaticFunction()
{
	return SecretValue;
}

TEST(HeartDynamicFunction, GlobalFunctionPointer)
{
	HeartDynamicFunction<int()> adapter1(&DynamicFunctionTestVisibleFunction);
	EXPECT_TRUE(adapter1);
	EXPECT_FALSE(!adapter1);
	EXPECT_TRUE(!!adapter1);

	int result = adapter1();
	EXPECT_EQ(result, SecretValue);
}

TEST(HeartDynamicFunction, DynamicFunctionTestStaticFunctionPointer)
{
	HeartDynamicFunction<int()> adapter1(&DynamicFunctionTestStaticFunction);
	EXPECT_TRUE(adapter1);
	EXPECT_FALSE(!adapter1);
	EXPECT_TRUE(!!adapter1);

	int result = adapter1();
	EXPECT_EQ(result, SecretValue);
}

TEST(HeartDynamicFunction, SwappableToEmpty)
{
	HeartDynamicFunction<int()> adapter1(&DynamicFunctionTestVisibleFunction);
	EXPECT_TRUE(adapter1);
	EXPECT_FALSE(!adapter1);
	EXPECT_TRUE(!!adapter1);

	int result1 = adapter1();
	EXPECT_EQ(result1, SecretValue);

	HeartDynamicFunction<int()> adapter2;
	EXPECT_FALSE(adapter2);
	EXPECT_TRUE(!adapter2);
	EXPECT_FALSE(!!adapter2);

	adapter1.Swap(adapter2);

	EXPECT_TRUE(adapter2);
	EXPECT_FALSE(adapter1);

	EXPECT_DEATH(adapter1(), ".*");

	int result2 = adapter2();
	EXPECT_EQ(result2, SecretValue);
	EXPECT_EQ(result1, result2);
}

TEST(HeartDynamicFunction, SwappableToFull)
{
	HeartDynamicFunction<int()> adapter1(&DynamicFunctionTestVisibleFunction);
	EXPECT_TRUE(adapter1);
	EXPECT_FALSE(!adapter1);
	EXPECT_TRUE(!!adapter1);

	int result1 = adapter1();
	EXPECT_EQ(result1, SecretValue);

	HeartDynamicFunction<int()> adapter2(&DynamicFunctionTestVisibleFunction);
	EXPECT_TRUE(adapter2);
	EXPECT_FALSE(!adapter2);
	EXPECT_TRUE(!!adapter2);

	adapter1.Swap(adapter2);

	EXPECT_TRUE(adapter2);
	EXPECT_TRUE(adapter1);

	int result2 = adapter2();
	int result3 = adapter1();
	EXPECT_EQ(result2, SecretValue);
	EXPECT_EQ(result3, SecretValue);
	EXPECT_EQ(result1, result2);
	EXPECT_EQ(result1, result3);
}

TEST(HeartDynamicFunction, ComplexSwap)
{
	static int moveCount = 0;

	struct UncopyableFunctor
	{
		DISABLE_COPY_SEMANTICS(UncopyableFunctor);

		int value;

		UncopyableFunctor() :
			value(0)
		{
		}

		UncopyableFunctor(UncopyableFunctor&& o) :
			value(SecretValue)
		{
			moveCount++;
		}

		int operator()() const
		{
			return SecretValue;
		}
	};

	HeartDynamicFunction<int()> adapter1(UncopyableFunctor {});
	EXPECT_EQ(moveCount, 1);
	EXPECT_TRUE(adapter1);
	EXPECT_FALSE(!adapter1);
	EXPECT_TRUE(!!adapter1);

	int result1 = adapter1();
	EXPECT_EQ(result1, SecretValue);

	HeartDynamicFunction<int()> adapter2(UncopyableFunctor {});
	EXPECT_EQ(moveCount, 2);
	EXPECT_TRUE(adapter1);
	EXPECT_FALSE(!adapter1);
	EXPECT_TRUE(!!adapter1);

	adapter1.Swap(adapter2);
	EXPECT_GE(moveCount, 2);

	EXPECT_TRUE(adapter1);
	EXPECT_TRUE(adapter2);

	int result2 = adapter2();
	int result3 = adapter1();
	EXPECT_EQ(result2, SecretValue);
	EXPECT_EQ(result3, SecretValue);
}

TEST(HeartDynamicFunction, Moveable)
{
	HeartDynamicFunction<int()> adapter1(&DynamicFunctionTestStaticFunction);
	EXPECT_TRUE(adapter1);

	HeartDynamicFunction<int()> adapter2;
	EXPECT_FALSE(adapter2);

	adapter2 = hrt::move(adapter1);

	EXPECT_FALSE(adapter1);
	EXPECT_DEATH(adapter1(), ".*");
	EXPECT_TRUE(adapter2);

	int result = adapter2();
	EXPECT_EQ(result, SecretValue);
}

TEST(HeartDynamicFunction, CapturelessLambda)
{
	HeartDynamicFunction<int()> adapter([]() {
		return SecretValue;
	});
	EXPECT_TRUE(adapter);
	EXPECT_FALSE(!adapter);
	EXPECT_TRUE(!!adapter);

	int result = adapter();
	EXPECT_EQ(result, SecretValue);
}

TEST(HeartDynamicFunction, StatefulLambda)
{
	int theSecret = SecretValue;

	HeartDynamicFunction<int()> adapter([theSecret]() {
		return theSecret;
	});

	EXPECT_TRUE(adapter);
	EXPECT_FALSE(!adapter);
	EXPECT_TRUE(!!adapter);

	int result = adapter();
	EXPECT_EQ(result, SecretValue);
}

TEST(HeartDynamicFunction, NoCopyParameter)
{
	struct NoCopyType
	{
		int v;

		NoCopyType() :
			v(SecretValue)
		{
		}

		NoCopyType(const NoCopyType&) = delete;
		NoCopyType& operator=(const NoCopyType&) = delete;
	};

	NoCopyType parameter;

	HeartDynamicFunction<int(const NoCopyType&)> constRefAdapter([](const NoCopyType& p) {
		return p.v;
	});

	EXPECT_TRUE(constRefAdapter);
	EXPECT_FALSE(!constRefAdapter);
	EXPECT_TRUE(!!constRefAdapter);

	int result = constRefAdapter(parameter);
	EXPECT_EQ(result, SecretValue);

	HeartDynamicFunction<int(NoCopyType&)> mutableRefAdapter([](NoCopyType& p) {
		p.v++;
		return p.v;
	});

	EXPECT_TRUE(mutableRefAdapter);
	EXPECT_FALSE(!mutableRefAdapter);
	EXPECT_TRUE(!!mutableRefAdapter);

	result = mutableRefAdapter(parameter);
	EXPECT_EQ(parameter.v, SecretValue + 1);
	EXPECT_EQ(result, SecretValue + 1);
}

TEST(HeartDynamicFunction, CopyableParameter)
{
	struct CopyableValue
	{
		int v;

		CopyableValue() :
			v(0)
		{
		}

		CopyableValue(const CopyableValue& v) :
			v(SecretValue)
		{
		}
	};

	CopyableValue parameter;
	EXPECT_EQ(parameter.v, 0);

	HeartDynamicFunction<int(CopyableValue)> adapter([](CopyableValue v) {
		EXPECT_EQ(v.v, SecretValue);
		return v.v;
	});

	EXPECT_TRUE(adapter);
	EXPECT_FALSE(!adapter);
	EXPECT_TRUE(!!adapter);

	int result = adapter(parameter);
	EXPECT_EQ(result, SecretValue);
}

TEST(HeartDynamicFunction, MoveOnlyReturnValue)
{
	struct MoveOnlyType
	{
		int v;

		MoveOnlyType() :
			v(0)
		{
		}

		MoveOnlyType(const MoveOnlyType&) = delete;

		MoveOnlyType(MoveOnlyType&& o) :
			v(SecretValue)
		{
			o.v = 0;
		}
	};

	HeartDynamicFunction<MoveOnlyType()> adapter([]() {
		MoveOnlyType aValue;
		EXPECT_EQ(aValue.v, 0);
		return std::move(aValue);
	});

	EXPECT_TRUE(adapter);
	EXPECT_FALSE(!adapter);
	EXPECT_TRUE(!!adapter);

	MoveOnlyType result = adapter();
	EXPECT_EQ(result.v, SecretValue);
}

TEST(HeartDynamicFunction, DifferentAllocators)
{
	struct LargeFunctor
	{
		uint32_t secret = 0;

		std::array<char, 512> data = {};

		uint32_t operator()()
		{
			return secret;
		}
	};

	struct SmallFunctor
	{
		uint32_t secret = 0;

		std::array<char, 32> data = {};

		uint32_t operator()()
		{
			return secret;
		}
	};

	TestTrackingAllocator allocatorA;
	TestTrackingAllocator allocatorB;

	HeartDynamicFunction<uint32_t()> adapterA(LargeFunctor {'A'}, allocatorA);
	HeartDynamicFunction<uint32_t()> adapterB(SmallFunctor {'B'}, allocatorB);

	EXPECT_GE(allocatorA.m_allocatedCount, 1);
	EXPECT_GE(allocatorA.m_allocatedSize, sizeof(LargeFunctor));
	EXPECT_GE(allocatorB.m_allocatedCount, 1);
	EXPECT_GE(allocatorB.m_allocatedSize, sizeof(SmallFunctor));

	uint64_t largeSize = allocatorA.m_allocatedSize;
	uint64_t smallSize = allocatorB.m_allocatedSize;

	EXPECT_TRUE(adapterA);
	EXPECT_EQ(adapterA(), 'A');
	EXPECT_TRUE(adapterB);
	EXPECT_EQ(adapterB(), 'B');

	adapterA.Swap(adapterB);

	EXPECT_EQ(allocatorA.m_allocatedSize, smallSize);
	EXPECT_EQ(allocatorB.m_allocatedSize, largeSize);

	EXPECT_TRUE(adapterA);
	EXPECT_EQ(adapterA(), 'B');
	EXPECT_TRUE(adapterB);
	EXPECT_EQ(adapterB(), 'A');

	adapterA.Clear();
	adapterB.Clear();

	EXPECT_EQ(allocatorA.m_allocatedCount, 0);
	EXPECT_EQ(allocatorB.m_allocatedCount, 0);
}
