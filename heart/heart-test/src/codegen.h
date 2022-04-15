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

#include <heart/codegen/codegen.h>

#include <heart/stl/vector.h>

#include <array>

SERIALIZE_STRUCT()
struct DeserializationTestStruct
{
	float floatValue = 0.0f;
	int intValue = 0;
	SerializedString<32> stringValue = {};
};

SERIALIZE_STRUCT()
struct NestedDeserializationTestStruct
{
	int outerIntValue = 0;

	SERIALIZE_AS_REF()
	DeserializationTestStruct testStruct;
};

SERIALIZE_STRUCT()
struct VectorDeserializationTestStruct
{
	SERIALIZE_AS_REF()
	hrt::vector<DeserializationTestStruct> values;
};

SERIALIZE_STRUCT()
struct ArrayDeserializationTestStruct
{
	SERIALIZE_AS_REF()
	std::array<DeserializationTestStruct, 4> values = {};
};
