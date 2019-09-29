#pragma once

#include <heart/debug/assert.h>
#include <heart/types.h>

#include <heart/stl/type_traits.h>

#include <entt/core/hashed_string.hpp>
#include <entt/meta/factory.hpp>
#include <entt/meta/meta.hpp>

#include <rapidjson/document.h>

template <size_t N>
struct SerializedString
{
	char buffer[N];

	static SerializedString SpecialConvert(const char* str)
	{
		SerializedString<N> result;
		result = str;
		return result;
	}

	void Set(const char* c)
	{
		strcpy_s(buffer, c);
	}

	const char* Get()
	{
		return buffer;
	}

	SerializedString& operator=(const char* str)
	{
		strcpy_s(buffer, str);
		return *this;
	}

	SerializedString(const char* str)
	{
		strcpy_s(buffer, str);
	}

	SerializedString()
	{
		// TODO: This isn't ideal... but entt requires that we only reflect a particular
		// conversion once :(
		static bool reflected = false;
		if (!reflected)
		{
			entt::reflect<const char*>().conv<SerializedString<N>::SpecialConvert>();
			reflected = true;
		}
	}
};

template <typename T>
struct is_entt_meta_any : hrt::false_type
{
};

template <>
struct is_entt_meta_any<entt::meta_any> : hrt::true_type
{
};

template <typename OutType, typename RapidjsonType>
bool DeserializeObject(OutType& outObject, RapidjsonType& node)
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

	if constexpr (is_entt_meta_any<hrt::remove_reference_t<decltype(outObject)>>::value)
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

		if (child.value.IsObject())
		{
			auto data = metaType.data(entt::hashed_string::to_value(child.name.GetString()));
			auto instance = data.get(outObject);
			if (!DeserializeObject(instance, child.value))
				return false;

			data.set(outObject, instance);
		}
		else if (child.value.IsString())
		{
			metaType.data(childId).set(outObject, child.value.GetString());
		}
		else if (child.value.IsBool())
		{
			metaType.data(childId).set(outObject, child.value.GetBool());
		}
		else if (child.value.IsInt())
		{
			metaType.data(childId).set(outObject, child.value.GetInt());
		}
		else if (child.value.IsUint())
		{
			metaType.data(childId).set(outObject, child.value.GetUint());
		}
		else if (child.value.IsFloat())
		{
			metaType.data(childId).set(outObject, child.value.GetFloat());
		}
	}

	return true;
}

#define BEGIN_SERIALIZE_TYPE(type_name) entt::reflect<type_name>(#type_name##_hs)
#define SERIALIZE_FIELD(type_name, field) .data<&type_name ::field>(#field##_hs)
#define END_SERIALIZE_TYPE(type_name) ;

#define SERIALIZE_STRUCT()
