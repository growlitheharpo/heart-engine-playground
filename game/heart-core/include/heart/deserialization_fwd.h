#pragma once

#ifndef __HEART_CODEGEN_ACTIVE
#define SERIALIZE_STRUCT()
#define SERIALIZE_AS_REF()
#define SERIALIZE_MEMBER_METHOD()
#define HIDE_FROM_CODEGEN(...) __VA_ARGS__
#else
#define SERIALIZE_STRUCT() void HEARTGEN___SERIALIZE_NEXT_SYMBOL_STRUCT();
#define SERIALIZE_AS_REF() void HEARTGEN___SERIALIZE_NEXT_SYMBOL_AS_REF();
#define SERIALIZE_MEMBER_METHOD() void HEARTGEN___SERIALIZE_NEXT_SYMBOL_AS_MEMB_FUNCTION();
#define HIDE_FROM_CODEGEN(...)
#endif

template <size_t N>
struct SerializedString
{
	char buffer[N] = {};

	static SerializedString CreateFromCString(const char* str)
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

	const char* c_str() const
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
	}
};

// If this changes, update SerializationGen in heart-codegen!
constexpr size_t SerializedDataPathSize = 64;
typedef SerializedString<SerializedDataPathSize> SerializedDataPath;
