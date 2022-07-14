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

#include <heart/config.h>
#include <heart/debug/assert.h>
#include <heart/types.h>

// TODO: including hrt::vector here causes types to be redefined as belonging to
//  std in heart-codegen if we parse cpp files... what the hell is happening?
#include <heart/stl/vector.h>

#include <heart/stl/type_traits.h>

#if HEART_HAS_ENTT
#include <entt/core/hashed_string.hpp>
#include <entt/meta/container.hpp>
#include <entt/meta/factory.hpp>
#include <entt/meta/meta.hpp>
#include <entt/meta/resolve.hpp>
#endif

#if HEART_HAS_RAPIDJSON
#include <rapidjson/document.h>
#endif

#if HEART_HAS_ENTT && HEART_HAS_RAPIDJSON
#define HEART_HAS_DESERIALIZATION_SUPPORT 1
#else
#define HEART_HAS_DESERIALIZATION_SUPPORT 0
#endif

#if HEART_HAS_DESERIALIZATION_SUPPORT

namespace entt
{
	template <typename Type, typename... Args>
	struct meta_sequence_container_traits<hrt::vector<Type, Args...>> :
		public internal::basic_meta_sequence_container_traits<hrt::vector<Type, Args...>>
	{
	};
}

namespace heart_priv
{
	template <typename OutType>
	static bool ReadSingleProperty(OutType& targetObject, entt::meta_data metaField, rapidjson::Value& jsonNode);
}

template <typename OutType, typename RapidjsonType>
bool HeartDeserializeObject(OutType& outObject, RapidjsonType& node)
{
	if (!HEART_CHECK(node.IsObject()))
		return false;

	auto nodeObj = node.GetObject();
	auto typeIter = nodeObj.FindMember("type");
	if (typeIter == nodeObj.MemberEnd() || !typeIter->value.IsString())
		return false;

	auto typeStr = typeIter->value.GetString();
	auto metaType = entt::resolve(entt::hashed_string::value(typeStr));
	if (!HEART_CHECK(metaType, "Type is not reflected!", typeStr))
		return false;

	if constexpr (hrt::is_same<entt::meta_any, hrt::remove_reference_t<decltype(outObject)>>::value)
	{
		if (!HEART_CHECK(outObject.type() == metaType, "DeserializeObject was passed meta_handle of the wrong type!"))
			return false;
	}

	auto contentsIter = nodeObj.FindMember("contents");
	if (contentsIter == nodeObj.MemberEnd() || !contentsIter->value.IsObject())
		return false;

	auto contents = contentsIter->value.GetObject();
	for (auto& child : contents)
	{
		HEART_CHECK(child.name.IsString());
		auto fieldId = entt::hashed_string::value(child.name.GetString());

		if (!heart_priv::ReadSingleProperty(outObject, metaType.data(fieldId), child.value))
			return false;
	}

	return true;
}

namespace heart_priv
{
	template <typename OutType>
	static bool ReadSingleProperty(OutType& targetObject, entt::meta_data metaField, rapidjson::Value& jsonNode)
	{
		using namespace entt::literals;

		if (jsonNode.IsObject())
		{
			// "Recurse" back into HeartDeserializeObject with a meta_any if
			// this is an object and let it determine type, fields, etc
			auto&& fieldInstance = metaField.get(targetObject);
			if (!HeartDeserializeObject(fieldInstance, jsonNode))
				return false;
		}
		else if (jsonNode.IsString())
		{
			metaField.set(targetObject, jsonNode.GetString());
		}
		else if (jsonNode.IsBool())
		{
			metaField.set(targetObject, jsonNode.GetBool());
		}
		else if (jsonNode.IsInt())
		{
			metaField.set(targetObject, jsonNode.GetInt());
		}
		else if (jsonNode.IsUint())
		{
			metaField.set(targetObject, jsonNode.GetUint());
		}
		else if (jsonNode.IsFloat())
		{
			metaField.set(targetObject, jsonNode.GetFloat());
		}
		else if (jsonNode.IsArray())
		{
			auto jsonArr = jsonNode.GetArray();
			auto targetAny = metaField.get(targetObject);

			// If the real type is an actual array, we can set by-index
			if (auto targetSequence = targetAny.as_sequence_container(); targetSequence)
			{
				// Attempt to resize, silently fail if it's fixed-size
				targetSequence.resize(jsonArr.Size());

				// Loop over min(jsonArr.Size(), targetSequence.size())
				for (rapidjson::SizeType i = 0; i < jsonArr.Size() && i < targetSequence.size(); ++i)
				{
					// Get the object at this index
					auto metaObject = targetSequence[i];

					if (jsonArr[i].IsObject())
					{
						// "Recurse" back into HeartDeserializeObject with a meta_any if
						// this is an object and let it determine type, fields, etc
						if (!HeartDeserializeObject(metaObject, jsonArr[i]))
							return false;
					}
					else if (auto metaSelf = metaObject.type().data("self"_hs); metaSelf)
					{
						// Use self reflection otherwise
						if (!ReadSingleProperty(metaObject, metaSelf, jsonArr[i]))
							return false;
					}
					else
					{
						HEART_ASSERT(false, "Type inside of array was not an object and did not have \"self\" reflected as data! You must provide one or the other!");
					}
				}
			}
		}

		return true;
	}
}

#define BEGIN_SERIALIZE_TYPE(type_name) entt::meta<type_name>().type(#type_name##_hs)
#define BEGIN_SERIALIZE_TYPE_ADDITIVE(type_name) entt::meta<type_name>()
#define SERIALIZE_SELF_ACCESS(type_name, setter, getter) .data<setter, getter>("self"_hs)
#define SERIALIZE_CONVERSION(type_name, convert) .conv<convert>()
#define SERIALIZE_FIELD(type_name, field) .data<&type_name ::field>(#field##_hs)
#define SERIALIZE_FUNCTION(type_name, function) .func<&type_name ::function>(#function##_hs)
#define SERIALIZE_FIELD_ALIAS(type_name, field) .data<&type_name ::field, entt::as_ref_t>(#field##_hs)
#define SERIALIZE_FUNCTION_ALIAS(type_name, function) .func<&type_name ::function, entt::as_ref_t>(#function##_hs)
#define END_SERIALIZE_TYPE(type_name) ;

#else

template <typename T1, typename T2>
bool HeartDeserializeObject(T1& outObject, T2& node)
{
	return false;
}

#define BEGIN_SERIALIZE_TYPE(type_name)
#define BEGIN_SERIALIZE_TYPE_ADDITIVE(type_name)
#define SERIALIZE_SELF_ACCESS(type_name, setter, getter)
#define SERIALIZE_CONVERSION(type_name, convert)
#define SERIALIZE_FIELD(type_name, field)
#define SERIALIZE_FUNCTION(type_name, function)
#define SERIALIZE_FIELD_ALIAS(type_name, field)
#define SERIALIZE_FUNCTION_ALIAS(type_name, function)
#define END_SERIALIZE_TYPE(type_name) ;

#endif
