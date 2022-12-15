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
#include <heart/memory/static_object.h>

#include <gtest/gtest.h>

template <int Value>
struct ObservableWrapper
{
	static inline bool Initialized = false;
};

struct Passkey
{
};

template <int Value>
struct SideEffectType
{
	SideEffectType(Passkey passkey)
	{
		ObservableWrapper<Value>::Initialized = true;
	}

	~SideEffectType()
	{
		ObservableWrapper<Value>::Initialized = false;
	}

	int ReturnProvidedValue(int value)
	{
		return value;
	}
};

TEST(StaticObject, LocalStaticDeclaration)
{
	constexpr int TestNumber = __COUNTER__;
	using Effect = ObservableWrapper<TestNumber>;
	using Cause = SideEffectType<TestNumber>;

	EXPECT_FALSE(Effect::Initialized);

	static HeartStaticObject<Cause> object;
	EXPECT_FALSE(Effect::Initialized);

	object.Construct(Passkey {});
	EXPECT_TRUE(Effect::Initialized);

	object.Destruct();
	EXPECT_FALSE(Effect::Initialized);
}

TEST(StaticObject, LocalDeclaration)
{
	constexpr int TestNumber = __COUNTER__;
	using Effect = ObservableWrapper<TestNumber>;
	using Cause = SideEffectType<TestNumber>;

	EXPECT_FALSE(Effect::Initialized);

	{
		HeartStaticObject<Cause> object;
		EXPECT_FALSE(Effect::Initialized);

		object.Construct(Passkey {});
		EXPECT_TRUE(Effect::Initialized);

		object.Destruct();
		EXPECT_FALSE(Effect::Initialized);
	}

	EXPECT_FALSE(Effect::Initialized);
}

TEST(StaticObject, LocalDeclarationWithScopeDestruct)
{
	constexpr int TestNumber = __COUNTER__;
	using Effect = ObservableWrapper<TestNumber>;
	using Cause = SideEffectType<TestNumber>;

	EXPECT_FALSE(Effect::Initialized);

	{
		HeartStaticObject<Cause> object;
		EXPECT_FALSE(Effect::Initialized);

		object.Construct(Passkey {});
		EXPECT_TRUE(Effect::Initialized);
	}

	EXPECT_FALSE(Effect::Initialized);
}

constexpr int GlobalDeclarationTestNumber = __COUNTER__;
using GlobalDeclarationEffect = ObservableWrapper<GlobalDeclarationTestNumber>;
using GlobalDeclarationCause = SideEffectType<GlobalDeclarationTestNumber>;

HeartStaticObject<GlobalDeclarationCause> GlobalObject;

TEST(StaticObject, GlobalDeclaration)
{
	EXPECT_FALSE(GlobalDeclarationEffect::Initialized);

	GlobalObject.Construct(Passkey {});
	EXPECT_TRUE(GlobalDeclarationEffect::Initialized);

	GlobalObject.Destruct();

	EXPECT_FALSE(GlobalDeclarationEffect::Initialized);
}

TEST(StaticObject, StaticConstruct)
{
	constexpr int TestNumber = __COUNTER__;
	using Effect = ObservableWrapper<TestNumber>;
	using Cause = SideEffectType<TestNumber>;

	EXPECT_FALSE(Effect::Initialized);

	static HeartStaticObject<Cause, HeartStaticObjectEnableStaticConstruct> object(Passkey {});
	EXPECT_TRUE(Effect::Initialized);
}

TEST(StaticObject, StaticConstructNoArgument)
{
	constexpr int TestNumber = __COUNTER__;
	using Effect = ObservableWrapper<TestNumber>;
	struct Cause
	{
		Cause()
		{
			Effect::Initialized = true;
		}

		~Cause()
		{
			Effect::Initialized = false;
		}
	};

	EXPECT_FALSE(Effect::Initialized);

	static HeartStaticObject<Cause, HeartStaticObjectEnableStaticConstruct> object;
	EXPECT_TRUE(Effect::Initialized);

	object.Destruct();
	EXPECT_FALSE(Effect::Initialized);
}

TEST(StaticObject, Access)
{
	static bool Created = false;
	struct ObjectWithData
	{
		int Value;

		ObjectWithData(int x) :
			Value(x)
		{
			Created = true;
		}
	};

	static HeartStaticObject<ObjectWithData> object;
	EXPECT_FALSE(Created);

	constexpr int SecretValue = 15;

	object.Construct(SecretValue);
	EXPECT_TRUE(Created);

	EXPECT_EQ(object->Value, SecretValue);
	EXPECT_EQ((*object).Value, SecretValue);
	EXPECT_EQ(object.Get().Value, SecretValue);
}
