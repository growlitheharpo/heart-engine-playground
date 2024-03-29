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
#include <heart/deserialization/deserialization.h>

static_assert(HEART_HAS_DESERIALIZATION_SUPPORT == 1, "HEART_HAS_DESERIALIZATION_SUPPORT is required to run codegen test.");

#include "codegen.h"
#include "gen/gen.h"

#include <rapidjson/document.h>

#include <entt/meta/container.hpp>

#include <gtest/gtest.h>

class HeartDeserializationFixture : public ::testing::Test
{
public:
	static void SetUpTestCase()
	{
		ReflectSerializedData();
	}
};

const char* BasicDeserializationTestJsonData = R"(
{ 
	"type": "DeserializationTestStruct",
	"contents": {
			"floatValue": 123.0,
			"intValue": 456,
			"stringValue": "789"
	}
})";

TEST_F(HeartDeserializationFixture, BasicDeserialization)
{
	rapidjson::Document doc;
	doc.Parse(BasicDeserializationTestJsonData);

	ASSERT_TRUE(!doc.HasParseError());

	DeserializationTestStruct target;
	EXPECT_EQ(target.floatValue, 0.0f);
	EXPECT_EQ(target.intValue, 0);
	EXPECT_EQ(strlen(target.stringValue.c_str()), 0);

	EXPECT_TRUE(HeartDeserializeObject(target, doc));

	EXPECT_EQ(target.floatValue, 123.0f);
	EXPECT_EQ(target.intValue, 456);
	EXPECT_EQ(strcmp(target.stringValue.c_str(), "789"), 0);
}

const char* NestedDeserializationTestJsonData = R"(
{
	"type": "NestedDeserializationTestStruct",
	"contents": {
		"outerIntValue": 123,
		"testStruct": {
			"type": "DeserializationTestStruct",
			"contents": {
				"floatValue": 456.0,
				"intValue": 789,
				"stringValue": "hello world"
			}
		}
	}
})";

TEST_F(HeartDeserializationFixture, NestedDeserialization)
{
	rapidjson::Document doc;
	doc.Parse(NestedDeserializationTestJsonData);

	ASSERT_TRUE(!doc.HasParseError());

	NestedDeserializationTestStruct target;
	EXPECT_EQ(target.outerIntValue, 0);
	EXPECT_EQ(target.testStruct.floatValue, 0.0f);
	EXPECT_EQ(target.testStruct.intValue, 0);
	EXPECT_EQ(strlen(target.testStruct.stringValue.c_str()), 0);

	EXPECT_TRUE(HeartDeserializeObject(target, doc));

	EXPECT_EQ(target.outerIntValue, 123);
	EXPECT_EQ(target.testStruct.floatValue, 456.0f);
	EXPECT_EQ(target.testStruct.intValue, 789);
	EXPECT_EQ(strcmp(target.testStruct.stringValue.c_str(), "hello world"), 0);
}

const char* VectorDeserializationTestJsonData = R"(
{
	"type": "VectorDeserializationTestStruct",
	"contents": {
		"values": [{
			"type": "DeserializationTestStruct",
			"contents": {
				"floatValue": 111.0,
				"intValue": 111,
				"stringValue": "hello world 1"
			}
		},{
			"type": "DeserializationTestStruct",
			"contents": {
				"floatValue": 222.0,
				"intValue": 222,
				"stringValue": "hello world 2"
			}
		},{
			"type": "DeserializationTestStruct",
			"contents": {
				"floatValue": 333.0,
				"intValue": 333,
				"stringValue": "hello world 3"
			}
		}]
	}
})";

TEST_F(HeartDeserializationFixture, VectorDeserialization)
{
	rapidjson::Document doc;
	doc.Parse(VectorDeserializationTestJsonData);

	ASSERT_TRUE(!doc.HasParseError());
	static_assert(entt::is_complete_v<entt::meta_sequence_container_traits<hrt::vector<DeserializationTestStruct>>>);

	VectorDeserializationTestStruct target;
	EXPECT_TRUE(target.values.empty());

	EXPECT_TRUE(HeartDeserializeObject(target, doc));

	ASSERT_EQ(target.values.size(), 3);
	EXPECT_EQ(target.values[0].floatValue, 111.0f);
	EXPECT_EQ(target.values[0].intValue, 111);
	EXPECT_EQ(strcmp(target.values[0].stringValue.c_str(), "hello world 1"), 0);

	EXPECT_EQ(target.values[1].floatValue, 222.0f);
	EXPECT_EQ(target.values[1].intValue, 222);
	EXPECT_EQ(strcmp(target.values[1].stringValue.c_str(), "hello world 2"), 0);

	EXPECT_EQ(target.values[2].floatValue, 333.0f);
	EXPECT_EQ(target.values[2].intValue, 333);
	EXPECT_EQ(strcmp(target.values[2].stringValue.c_str(), "hello world 3"), 0);
}

const char* ArrayDeserializationTestJsonData = R"(
{
	"type": "ArrayDeserializationTestStruct",
	"contents": {
		"values": [{
			"type": "DeserializationTestStruct",
			"contents": {
				"floatValue": 111.0,
				"intValue": 111,
				"stringValue": "hello world 1"
			}
		},{
			"type": "DeserializationTestStruct",
			"contents": {
				"floatValue": 222.0,
				"intValue": 222,
				"stringValue": "hello world 2"
			}
		},{
			"type": "DeserializationTestStruct",
			"contents": {
				"floatValue": 333.0,
				"intValue": 333,
				"stringValue": "hello world 3"
			}
		}]
	}
})";

TEST_F(HeartDeserializationFixture, ArrayDeserialization)
{
	rapidjson::Document doc;
	doc.Parse(ArrayDeserializationTestJsonData);

	ASSERT_TRUE(!doc.HasParseError());

	ArrayDeserializationTestStruct target;
	for (auto& entry : target.values)
	{
		EXPECT_EQ(entry.floatValue, 0.0f);
		EXPECT_EQ(entry.intValue, 0);
		EXPECT_EQ(strlen(entry.stringValue.c_str()), 0);
	}

	EXPECT_TRUE(HeartDeserializeObject(target, doc));

	EXPECT_EQ(target.values[0].floatValue, 111.0f);
	EXPECT_EQ(target.values[0].intValue, 111);
	EXPECT_EQ(strcmp(target.values[0].stringValue.c_str(), "hello world 1"), 0);

	EXPECT_EQ(target.values[1].floatValue, 222.0f);
	EXPECT_EQ(target.values[1].intValue, 222);
	EXPECT_EQ(strcmp(target.values[1].stringValue.c_str(), "hello world 2"), 0);

	EXPECT_EQ(target.values[2].floatValue, 333.0f);
	EXPECT_EQ(target.values[2].intValue, 333);
	EXPECT_EQ(strcmp(target.values[2].stringValue.c_str(), "hello world 3"), 0);

	EXPECT_EQ(target.values[3].floatValue, 0.0f);
	EXPECT_EQ(target.values[3].intValue, 0);
	EXPECT_EQ(strlen(target.values[3].stringValue.c_str()), 0);
}
