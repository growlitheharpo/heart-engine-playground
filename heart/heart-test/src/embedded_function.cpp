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
#include <heart/function/embedded_function.h>

#include <heart/types.h>

#include <gtest/gtest.h>

constexpr int SecretValue = 0xF00DFEED;

int VisibleFunction()
{
	return SecretValue;
}

static int StaticFunction()
{
	return SecretValue;
}

TEST(HeartEmbeddedFunction, GlobalFunctionPointer)
{
	HeartEmbeddedFunction<int()> adapter1(&VisibleFunction);
	EXPECT_TRUE(adapter1);
	EXPECT_FALSE(!adapter1);
	EXPECT_TRUE(!!adapter1);

	int result = adapter1();
	EXPECT_EQ(result, SecretValue);
}

TEST(HeartEmbeddedFunction, StaticFunctionPointer)
{
	HeartEmbeddedFunction<int()> adapter1(&StaticFunction);
	EXPECT_TRUE(adapter1);
	EXPECT_FALSE(!adapter1);
	EXPECT_TRUE(!!adapter1);

	int result = adapter1();
	EXPECT_EQ(result, SecretValue);
}

TEST(HeartEmbeddedFunction, SwappableToEmpty)
{
	HeartEmbeddedFunction<int()> adapter1(&VisibleFunction);
	EXPECT_TRUE(adapter1);
	EXPECT_FALSE(!adapter1);
	EXPECT_TRUE(!!adapter1);

	int result1 = adapter1();
	EXPECT_EQ(result1, SecretValue);

	HeartEmbeddedFunction<int()> adapter2;
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

TEST(HeartEmbeddedFunction, SwappableToFull)
{
	HeartEmbeddedFunction<int()> adapter1(&VisibleFunction);
	EXPECT_TRUE(adapter1);
	EXPECT_FALSE(!adapter1);
	EXPECT_TRUE(!!adapter1);

	int result1 = adapter1();
	EXPECT_EQ(result1, SecretValue);

	HeartEmbeddedFunction<int()> adapter2(&VisibleFunction);
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

TEST(HeartEmbeddedFunction, ComplexSwap)
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

	HeartEmbeddedFunction<int()> adapter1(UncopyableFunctor {});
	EXPECT_EQ(moveCount, 1);
	EXPECT_TRUE(adapter1);
	EXPECT_FALSE(!adapter1);
	EXPECT_TRUE(!!adapter1);

	int result1 = adapter1();
	EXPECT_EQ(result1, SecretValue);

	HeartEmbeddedFunction<int()> adapter2(UncopyableFunctor {});
	EXPECT_EQ(moveCount, 2);
	EXPECT_TRUE(adapter1);
	EXPECT_FALSE(!adapter1);
	EXPECT_TRUE(!!adapter1);

	adapter1.Swap(adapter2);
	EXPECT_GE(moveCount, 4);

	EXPECT_TRUE(adapter1);
	EXPECT_TRUE(adapter2);

	int result2 = adapter2();
	int result3 = adapter1();
	EXPECT_EQ(result2, SecretValue);
	EXPECT_EQ(result3, SecretValue);
}

TEST(HeartEmbeddedFunction, Moveable)
{
	HeartEmbeddedFunction<int()> adapter1(&StaticFunction);
	EXPECT_TRUE(adapter1);

	HeartEmbeddedFunction<int()> adapter2;
	EXPECT_FALSE(adapter2);

	adapter2 = hrt::move(adapter1);

	EXPECT_FALSE(adapter1);
	EXPECT_DEATH(adapter1(), ".*");
	EXPECT_TRUE(adapter2);

	int result = adapter2();
	EXPECT_EQ(result, SecretValue);
}

TEST(HeartEmbeddedFunction, CapturelessLambda)
{
	HeartEmbeddedFunction<int()> adapter([]() {
		return SecretValue;
	});
	EXPECT_TRUE(adapter);
	EXPECT_FALSE(!adapter);
	EXPECT_TRUE(!!adapter);

	int result = adapter();
	EXPECT_EQ(result, SecretValue);
}

TEST(HeartEmbeddedFunction, StatefulLambda)
{
	int theSecret = SecretValue;

	HeartEmbeddedFunction<int()> adapter([theSecret]() {
		return theSecret;
	});

	EXPECT_TRUE(adapter);
	EXPECT_FALSE(!adapter);
	EXPECT_TRUE(!!adapter);

	int result = adapter();
	EXPECT_EQ(result, SecretValue);
}

TEST(HeartEmbeddedFunction, NoCopyParameter)
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

	HeartEmbeddedFunction<int(const NoCopyType&)> constRefAdapter([](const NoCopyType& p) {
		return p.v;
	});

	EXPECT_TRUE(constRefAdapter);
	EXPECT_FALSE(!constRefAdapter);
	EXPECT_TRUE(!!constRefAdapter);

	int result = constRefAdapter(parameter);
	EXPECT_EQ(result, SecretValue);

	HeartEmbeddedFunction<int(NoCopyType&)> mutableRefAdapter([](NoCopyType& p) {
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

TEST(HeartEmbeddedFunction, CopyableParameter)
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

	HeartEmbeddedFunction<int(CopyableValue)> adapter([](CopyableValue v) {
		EXPECT_EQ(v.v, SecretValue);
		return v.v;
	});

	EXPECT_TRUE(adapter);
	EXPECT_FALSE(!adapter);
	EXPECT_TRUE(!!adapter);

	int result = adapter(parameter);
	EXPECT_EQ(result, SecretValue);
}

TEST(HeartEmbeddedFunction, MoveOnlyReturnValue)
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

	HeartEmbeddedFunction<MoveOnlyType()> adapter([]() {
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

TEST(HeartEmbeddedFunction, BigStorage)
{
	struct BigFunctor
	{
		char big[1024];

		int operator()() const
		{
			return SecretValue + int(sizeof(big));
		}
	};

	// Ensure it (correctly) fails to compile for large storage
#if 0
	HeartEmbeddedFunction<int(), 32> adapter(BigFunctor {});
#endif

	// Ensure it (correctly) fails to compile for something that *seems* like it should fit
	// This fails because it doesn't include room for the vtable
#if 0
	HeartEmbeddedFunction<int(), sizeof(BigFunctor)> adapter(BigFunctor {});
#endif

	HeartEmbeddedFunction<int(), 2048> bigAdapter(BigFunctor {});
	EXPECT_TRUE(bigAdapter);

	int result = bigAdapter();
	EXPECT_EQ(result, SecretValue + sizeof(BigFunctor::big));
}
