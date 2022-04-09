#include <heart/hash/murmur.h>

#include <gtest/gtest.h>

#include <smhasher/src/MurmurHash3.h>

constexpr uint32_t CompileTimeTest = HeartMurmurHash3("Heart Murmurhash Compile Time Test");

TEST(MurmurHash, CompileTimeString)
{
	std::string_view str("Heart Murmurhash Compile Time Test");

	uint32_t theirResult = 0;
	MurmurHash3_x86_32(str.data(), int(str.size()), HeartMurmurHash3DefaultSeed, &theirResult);

	EXPECT_EQ(theirResult, CompileTimeTest);
}

TEST(MurmurHash, int8)
{
	int8_t dataBlock[] = {116, 50, 95, -66, 83, 82, 16, 113, -84, -98, -68, -126, -87};

	uint32_t ourResult = HeartMurmurHash3(dataBlock, sizeof(dataBlock));

	uint32_t theirResult = 0;
	MurmurHash3_x86_32(dataBlock, int(sizeof(dataBlock)), HeartMurmurHash3DefaultSeed, &theirResult);

	EXPECT_EQ(ourResult, theirResult);
}

TEST(MurmurHash, uint8)
{
	uint8_t dataBlock[] = {206, 247, 131, 148, 48, 202, 56, 45, 14, 148, 66, 179, 246};

	uint32_t ourResult = HeartMurmurHash3(dataBlock, sizeof(dataBlock));

	uint32_t theirResult = 0;
	MurmurHash3_x86_32(dataBlock, int(sizeof(dataBlock)), HeartMurmurHash3DefaultSeed, &theirResult);

	EXPECT_EQ(ourResult, theirResult);
}

TEST(MurmurHash, char)
{
	char dataBlock[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd'};

	uint32_t ourResult = HeartMurmurHash3(dataBlock, sizeof(dataBlock));

	uint32_t theirResult = 0;
	MurmurHash3_x86_32(dataBlock, int(sizeof(dataBlock)), HeartMurmurHash3DefaultSeed, &theirResult);

	EXPECT_EQ(ourResult, theirResult);
}

TEST(MurmurHash, int16)
{
	int16_t dataBlock[] = {1630, 14322, -3077, 13559, 7146, 9661, 7298, 7792, -14984, 7571, -8883, 5025, 9336};

	uint32_t ourResult = HeartMurmurHash3(dataBlock, sizeof(dataBlock) / sizeof(int16_t));

	uint32_t theirResult = 0;
	MurmurHash3_x86_32(dataBlock, int(sizeof(dataBlock)), HeartMurmurHash3DefaultSeed, &theirResult);

	EXPECT_EQ(ourResult, theirResult);
}

TEST(MurmurHash, uint16)
{
	uint16_t dataBlock[] = {20399, 19899, 31692, 13863, 16941, 11613, 14892, 746, 10620, 7331, 31468, 26272, 14066};

	uint32_t ourResult = HeartMurmurHash3(dataBlock, sizeof(dataBlock) / sizeof(uint16_t));

	uint32_t theirResult = 0;
	MurmurHash3_x86_32(dataBlock, int(sizeof(dataBlock)), HeartMurmurHash3DefaultSeed, &theirResult);

	EXPECT_EQ(ourResult, theirResult);
}

TEST(MurmurHash, int32)
{
	int32_t dataBlock[] = {1654508, 876206, -1949660, 2009014, -1798597, -523854, -2529957, 924645, -1010157, 863839, 1860515, 2457578, 650635};

	uint32_t ourResult = HeartMurmurHash3(dataBlock, sizeof(dataBlock) / sizeof(int32_t));

	uint32_t theirResult = 0;
	MurmurHash3_x86_32(dataBlock, int(sizeof(dataBlock)), HeartMurmurHash3DefaultSeed, &theirResult);

	EXPECT_EQ(ourResult, theirResult);
}

TEST(MurmurHash, uint32)
{
	uint32_t dataBlock[] = {136737210, 139783947, 427530759, 131542, 381706335, 140753215, 428140249, 187343278, 90080884, 3900185, 366672134, 84169969, 193066025};

	uint32_t ourResult = HeartMurmurHash3(dataBlock, sizeof(dataBlock) / sizeof(uint32_t));

	uint32_t theirResult = 0;
	MurmurHash3_x86_32(dataBlock, int(sizeof(dataBlock)), HeartMurmurHash3DefaultSeed, &theirResult);

	EXPECT_EQ(ourResult, theirResult);
}

TEST(MurmurHash, uint64)
{
	uint64_t dataBlock[] = {0xFAFFBAFFBEEF4B1D, 0xCAFEBABEDEADBEEF, 0xFACEFACEFACEFACE, 0xDEADC0DE0FF1CE};

	uint32_t ourResult = HeartMurmurHash3(dataBlock, sizeof(dataBlock) / sizeof(uint64_t));

	uint32_t theirResult = 0;
	MurmurHash3_x86_32(dataBlock, int(sizeof(dataBlock)), HeartMurmurHash3DefaultSeed, &theirResult);

	EXPECT_EQ(ourResult, theirResult);
}

TEST(MurmurHash, int64)
{
	int64_t dataBlock[] = {std::bit_cast<int64_t>(0xFAFFBAFFBEEF4B1D), std::bit_cast<int64_t>(0xCAFEBABEDEADBEEF), std::bit_cast<int64_t>(0xFACEFACEFACEFACE), std::bit_cast<int64_t>(0xDEADC0DE0FF1CE21)};

	uint32_t ourResult = HeartMurmurHash3(dataBlock, sizeof(dataBlock) / sizeof(int64_t));

	uint32_t theirResult = 0;
	MurmurHash3_x86_32(dataBlock, int(sizeof(dataBlock)), HeartMurmurHash3DefaultSeed, &theirResult);

	EXPECT_EQ(ourResult, theirResult);
}

TEST(MurmurHash, structure)
{
	struct MyWeirdStruct
	{
		uint64_t value1 = 0xFACEFACEFACEFACE;
		char value2 = 's';
		uint32_t value3 = 4275307592;
	};

	static_assert(sizeof(MyWeirdStruct) > 8);

	MyWeirdStruct value;

	uint32_t ourResult = HeartMurmurHash3(&value, 1);

	uint32_t theirResult = 0;
	MurmurHash3_x86_32(&value, sizeof(value), HeartMurmurHash3DefaultSeed, &theirResult);

	EXPECT_EQ(ourResult, theirResult);
}
