#include <heart/hash/string_hash.h>

#include <gtest/gtest.h>

TEST(HeartConstStringHash, ConstString)
{
	constexpr const char* CompileTimeString = "This is a static string";
	constexpr HeartConstStringHash compileHash(CompileTimeString);
	HeartConstStringHash runtimeHash(CompileTimeString);

	// Compile time check
	static_assert(compileHash.Value() == HeartMurmurHash3(CompileTimeString));
	static_assert(compileHash.Valid());

	// Runtime check
	EXPECT_EQ(runtimeHash.Value(), HeartMurmurHash3(CompileTimeString));
	EXPECT_TRUE(runtimeHash.Valid());

	// Equality
	EXPECT_EQ(compileHash.Value(), runtimeHash.Value());

#if HEART_INCLUDE_DEBUG_STRINGS
	constexpr std::string_view view = compileHash.DebugString();

	// Compile time check
	static_assert(std::string_view(CompileTimeString) == view);

	// Runtime check
	EXPECT_EQ(std::string_view(CompileTimeString), compileHash.DebugString());
	EXPECT_EQ(std::string_view(CompileTimeString), view);
	EXPECT_EQ(std::string_view(CompileTimeString), runtimeHash.DebugString());
#endif
}

TEST(HeartConstStringHash, EmptyString)
{
	constexpr HeartConstStringHash compileHash;
	HeartConstStringHash runtimeHash;

	// Compile time check
	static_assert(compileHash.Value() != HeartMurmurHash3("Valid string"));
	static_assert(compileHash.Value() == HeartInvalidStringHash);
	static_assert(!compileHash.Valid());

	// Runtime check
	EXPECT_EQ(runtimeHash.Value(), HeartInvalidStringHash);
	EXPECT_FALSE(runtimeHash.Valid());

	// Equality
	EXPECT_EQ(compileHash.Value(), runtimeHash.Value());

#if HEART_INCLUDE_DEBUG_STRINGS
	constexpr std::string_view view = compileHash.DebugString();

	// Compile time check
	static_assert(std::string_view {} == view);

	// Runtime check
	EXPECT_EQ(std::string_view {}, compileHash.DebugString());
	EXPECT_EQ(std::string_view {}, view);
	EXPECT_EQ(std::string_view {}, runtimeHash.DebugString());
#endif
}

TEST(HeartStringHash, String)
{
	constexpr const char* CompileTimeString = "This is a static string";
	constexpr HeartStringHash compileHash(CompileTimeString);
	HeartStringHash runtimeHash(CompileTimeString);

	// Compile time check
	static_assert(compileHash.Value() == HeartMurmurHash3(CompileTimeString));
	static_assert(compileHash.Valid());

	// Runtime check
	EXPECT_EQ(runtimeHash.Value(), HeartMurmurHash3(CompileTimeString));
	EXPECT_TRUE(runtimeHash.Valid());

	// Equality
	EXPECT_EQ(compileHash.Value(), runtimeHash.Value());

#if HEART_INCLUDE_DEBUG_STRINGS
	constexpr std::string_view view = compileHash.DebugString();

	// Compile time check
	static_assert(std::string_view(CompileTimeString) == view);

	// Runtime check
	EXPECT_EQ(std::string_view(CompileTimeString), compileHash.DebugString());
	EXPECT_EQ(std::string_view(CompileTimeString), view);
	EXPECT_EQ(std::string_view(CompileTimeString), runtimeHash.DebugString());
#endif
}

TEST(HeartStringHash, EmptyString)
{
	constexpr HeartStringHash compileHash;
	HeartStringHash runtimeHash;

	// Compile time check
	static_assert(compileHash.Value() != HeartMurmurHash3("Valid string"));
	static_assert(compileHash.Value() == HeartInvalidStringHash);
	static_assert(!compileHash.Valid());

	// Runtime check
	EXPECT_EQ(runtimeHash.Value(), HeartInvalidStringHash);
	EXPECT_FALSE(runtimeHash.Valid());

	// Equality
	EXPECT_EQ(compileHash.Value(), runtimeHash.Value());

#if HEART_INCLUDE_DEBUG_STRINGS
	constexpr std::string_view view = compileHash.DebugString();

	// Compile time check
	static_assert(std::string_view {} == view);

	// Runtime check
	EXPECT_EQ(std::string_view {}, compileHash.DebugString());
	EXPECT_EQ(std::string_view {}, view);
	EXPECT_EQ(std::string_view {}, runtimeHash.DebugString());
#endif
}

TEST(HeartStringHash, Interop)
{
	constexpr const char* CompileTimeString = "This is a static string";
	constexpr HeartConstStringHash compileConstHash(CompileTimeString);
	constexpr HeartStringHash compileHash(compileConstHash);

	HeartConstStringHash runtimeConstHash(CompileTimeString);
	HeartStringHash runtimeHash(runtimeConstHash);

	// Compile time check
	static_assert(compileConstHash.Valid());
	static_assert(compileHash.Valid());
	static_assert(compileConstHash.Value() == compileHash.Value());

	// Runtime check
	EXPECT_TRUE(runtimeConstHash.Valid());
	EXPECT_TRUE(runtimeHash.Valid());
	EXPECT_EQ(runtimeHash.Value(), runtimeConstHash.Value());

#if HEART_INCLUDE_DEBUG_STRINGS
	constexpr std::string_view view = CompileTimeString;

	// Compile time check
	static_assert(compileConstHash.DebugString() == view);
	static_assert(compileHash.DebugString() == view);
	static_assert(compileConstHash.DebugString() == compileHash.DebugString());

	// Runtime check
	EXPECT_EQ(runtimeConstHash.DebugString(), runtimeHash.DebugString());
#endif
}

TEST(HeartConstStringHash, Interop)
{
	constexpr const char* CompileTimeString = "This is a static string";
	constexpr HeartStringHash compileHash(CompileTimeString);
	constexpr HeartConstStringHash compileConstHash(compileHash);

	HeartConstStringHash runtimeConstHash(CompileTimeString);
	HeartStringHash runtimeHash(runtimeConstHash);

	// Compile time check
	static_assert(compileConstHash.Valid());
	static_assert(compileHash.Valid());
	static_assert(compileConstHash.Value() == compileHash.Value());

	// Runtime check
	EXPECT_TRUE(runtimeConstHash.Valid());
	EXPECT_TRUE(runtimeHash.Valid());
	EXPECT_EQ(runtimeHash.Value(), runtimeConstHash.Value());

#if HEART_INCLUDE_DEBUG_STRINGS
	constexpr std::string_view view = CompileTimeString;

	// Compile time check
	static_assert(compileConstHash.DebugString() == view);
	static_assert(compileHash.DebugString() == view);
	static_assert(compileConstHash.DebugString() == compileHash.DebugString());

	// Runtime check
	EXPECT_EQ(runtimeConstHash.DebugString(), runtimeHash.DebugString());
#endif
}
