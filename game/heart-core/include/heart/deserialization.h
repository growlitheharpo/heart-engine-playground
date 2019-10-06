#pragma once

#include <heart/debug/assert.h>
#include <heart/deserialization_fwd.h>
#include <heart/types.h>

#include <heart/stl/type_traits.h>

#include <entt/core/hashed_string.hpp>
#include <entt/meta/factory.hpp>
#include <entt/meta/meta.hpp>

#include <rapidjson/document.h>

namespace heart_priv
{
	template <typename OutType, typename MetaType, typename RapidjsonType>
	static bool ReadSingleProperty(OutType& outObject, MetaType& metaData, RapidjsonType& jsonNode);
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
	auto metaType = entt::resolve(entt::hashed_string::to_value(typeStr));
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
		auto childId = entt::hashed_string::to_value(child.name.GetString());

		if (!heart_priv::ReadSingleProperty(outObject, metaType.data(childId), child.value))
			return false;
	}

	return true;
}


namespace heart_priv
{
	template <typename OutType, typename MetaType, typename RapidjsonType>
	static bool ReadSingleProperty(OutType& outObject, MetaType& metaData, RapidjsonType& jsonNode, size_t index)
	{
		if (jsonNode.IsObject())
		{
			auto& instance = metaData.get(outObject, index);
			if (!HeartDeserializeObject(instance, jsonNode))
				return false;
		}
		else if (jsonNode.IsString())
		{
			metaData.set(outObject, index, jsonNode.GetString());
		}
		else if (jsonNode.IsBool())
		{
			metaData.set(outObject, index, jsonNode.GetBool());
		}
		else if (jsonNode.IsInt())
		{
			metaData.set(outObject, index, jsonNode.GetInt());
		}
		else if (jsonNode.IsUint())
		{
			metaData.set(outObject, index, jsonNode.GetUint());
		}
		else if (jsonNode.IsFloat())
		{
			metaData.set(outObject, index, jsonNode.GetFloat());
		}
		else if (jsonNode.IsArray())
		{
			HEART_ASSERT(false, "Heart Deserialization cannot parse 2D arrays!");
			return false;
		}

		return true;
	}

	template <typename OutType, typename MetaType, typename RapidjsonType>
	static bool ReadSingleProperty(OutType& outObject, MetaType& metaData, RapidjsonType& jsonNode)
	{
		if (jsonNode.IsObject())
		{
			auto& instance = metaData.get(outObject);
			if (!HeartDeserializeObject(instance, jsonNode))
				return false;
		}
		else if (jsonNode.IsString())
		{
			metaData.set(outObject, jsonNode.GetString());
		}
		else if (jsonNode.IsBool())
		{
			metaData.set(outObject, jsonNode.GetBool());
		}
		else if (jsonNode.IsInt())
		{
			metaData.set(outObject, jsonNode.GetInt());
		}
		else if (jsonNode.IsUint())
		{
			metaData.set(outObject, jsonNode.GetUint());
		}
		else if (jsonNode.IsFloat())
		{
			metaData.set(outObject, jsonNode.GetFloat());
		}
		else if (jsonNode.IsArray())
		{
			auto jsonArr = jsonNode.GetArray();
			for (rapidjson::SizeType i = 0; i < jsonArr.Size(); ++i)
			{
				if (!ReadSingleProperty(outObject, metaData, jsonArr[i], size_t(i)))
					return false;
			}
		}

		return true;
	}
}

#define BEGIN_SERIALIZE_TYPE(type_name) entt::reflect<type_name>(#type_name##_hs)
#define SERIALIZE_FIELD(type_name, field) .data<&type_name ::field>(#field##_hs)
#define SERIALIZE_FIELD_ALIAS(type_name, field) .data<&type_name ::field, entt::as_alias_t>(#field##_hs)
#define END_SERIALIZE_TYPE(type_name) ;
