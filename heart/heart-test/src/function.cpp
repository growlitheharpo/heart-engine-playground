#include <heart/function.h>

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

TEST(HeartFunction, GlobalFunctionPointer)
{
	HeartFunction<int()> adapter1(&VisibleFunction);
	EXPECT_TRUE(adapter1);
	EXPECT_FALSE(!adapter1);
	EXPECT_TRUE(!!adapter1);

	int result = adapter1();
	EXPECT_EQ(result, SecretValue);
}

TEST(HeartFunction, StaticFunctionPointer)
{
	HeartFunction<int()> adapter1(&StaticFunction);
	EXPECT_TRUE(adapter1);
	EXPECT_FALSE(!adapter1);
	EXPECT_TRUE(!!adapter1);

	int result = adapter1();
	EXPECT_EQ(result, SecretValue);
}

TEST(HeartFunction, SwappableToEmpty)
{
	HeartFunction<int()> adapter1(&VisibleFunction);
	EXPECT_TRUE(adapter1);
	EXPECT_FALSE(!adapter1);
	EXPECT_TRUE(!!adapter1);

	int result1 = adapter1();
	EXPECT_EQ(result1, SecretValue);

	HeartFunction<int()> adapter2;
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

TEST(HeartFunction, SwappableToFull)
{
	HeartFunction<int()> adapter1(&VisibleFunction);
	EXPECT_TRUE(adapter1);
	EXPECT_FALSE(!adapter1);
	EXPECT_TRUE(!!adapter1);

	int result1 = adapter1();
	EXPECT_EQ(result1, SecretValue);

	HeartFunction<int()> adapter2(&VisibleFunction);
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

TEST(HeartFunction, ComplexSwap)
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

	HeartFunction<int()> adapter1(UncopyableFunctor {});
	EXPECT_EQ(moveCount, 1);
	EXPECT_TRUE(adapter1);
	EXPECT_FALSE(!adapter1);
	EXPECT_TRUE(!!adapter1);

	int result1 = adapter1();
	EXPECT_EQ(result1, SecretValue);

	HeartFunction<int()> adapter2(UncopyableFunctor {});
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

TEST(HeartFunction, Moveable)
{
	HeartFunction<int()> adapter1(&StaticFunction);
	EXPECT_TRUE(adapter1);

	HeartFunction<int()> adapter2;
	EXPECT_FALSE(adapter2);

	adapter2 = hrt::move(adapter1);

	EXPECT_FALSE(adapter1);
	EXPECT_DEATH(adapter1(), ".*");
	EXPECT_TRUE(adapter2);

	int result = adapter2();
	EXPECT_EQ(result, SecretValue);
}

TEST(HeartFunction, CapturelessLambda)
{
	HeartFunction<int()> adapter([]() {
		return SecretValue;
	});
	EXPECT_TRUE(adapter);
	EXPECT_FALSE(!adapter);
	EXPECT_TRUE(!!adapter);

	int result = adapter();
	EXPECT_EQ(result, SecretValue);
}

TEST(HeartFunction, StatefulLambda)
{
	int theSecret = SecretValue;

	HeartFunction<int()> adapter([theSecret]() {
		return theSecret;
	});

	EXPECT_TRUE(adapter);
	EXPECT_FALSE(!adapter);
	EXPECT_TRUE(!!adapter);

	int result = adapter();
	EXPECT_EQ(result, SecretValue);
}

TEST(HeartFunction, NoCopyParameter)
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

	HeartFunction<int(const NoCopyType&)> constRefAdapter([](const NoCopyType& p) {
		return p.v;
	});

	EXPECT_TRUE(constRefAdapter);
	EXPECT_FALSE(!constRefAdapter);
	EXPECT_TRUE(!!constRefAdapter);

	int result = constRefAdapter(parameter);
	EXPECT_EQ(result, SecretValue);

	HeartFunction<int(NoCopyType&)> mutableRefAdapter([](NoCopyType& p) {
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

TEST(HeartFunction, CopyableParameter)
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

	HeartFunction<int(CopyableValue)> adapter([](CopyableValue v) {
		EXPECT_EQ(v.v, SecretValue);
		return v.v;
	});

	EXPECT_TRUE(adapter);
	EXPECT_FALSE(!adapter);
	EXPECT_TRUE(!!adapter);

	int result = adapter(parameter);
	EXPECT_EQ(result, SecretValue);
}

TEST(HeartFunction, MoveOnlyReturnValue)
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

	HeartFunction<MoveOnlyType()> adapter([]() {
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

TEST(HeartFunction, BigStorage)
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
	HeartFunction<int(), 32> adapter(BigFunctor {});
#endif

	// Ensure it (correctly) fails to compile for something that *seems* like it should fit
	// This fails because it doesn't include room for the vtable
#if 0
	HeartFunction<int(), sizeof(BigFunctor)> adapter(BigFunctor {});
#endif

	HeartFunction<int(), 2048> bigAdapter(BigFunctor {});
	EXPECT_TRUE(bigAdapter);

	int result = bigAdapter();
	EXPECT_EQ(result, SecretValue + sizeof(BigFunctor::big));
}
