#pragma once

#include <heart/types.h>

#include <heart/stl/type_traits/enable_if.h>

#include <bit>
#include <string_view>

namespace heart_priv
{
	template <typename T, typename Enable = void>
	struct HeartMurmurHashHelper;

	template <typename T>
	struct HeartMurmurHashHelper<T, hrt::enable_if_t<sizeof(T) == 1>>
	{
		const T* const buffer;
		const size_t size;

		constexpr HeartMurmurHashHelper(const T* const b, const size_t s) :
			buffer(b), size(s)
		{
		}

		constexpr uint32_t GetBlock32(int index) const
		{
			int i = index * 4;
			return uint32_t(std::bit_cast<uint8_t>(buffer[i + 0])) |
				   uint32_t(std::bit_cast<uint8_t>(buffer[i + 1])) << 8u |
				   uint32_t(std::bit_cast<uint8_t>(buffer[i + 2])) << 16u |
				   uint32_t(std::bit_cast<uint8_t>(buffer[i + 3])) << 24u;
		}

		constexpr uint32_t GetTail(int tail) const
		{
			const int tailSize = size % 4;
			return uint32_t(std::bit_cast<uint8_t>(buffer[size - tailSize + tail]));
		}
	};

	template <typename T>
	struct HeartMurmurHashHelper<T, hrt::enable_if_t<sizeof(T) == 2>>
	{
		const T* const buffer;
		const size_t size;

		constexpr HeartMurmurHashHelper(const T* const b, const size_t s) :
			buffer(b), size(s)
		{
		}

		constexpr uint32_t GetBlock32(int index) const
		{
			int i = index * 2;
			return uint32_t(std::bit_cast<uint16_t>(buffer[i + 0])) |
				   uint32_t(std::bit_cast<uint16_t>(buffer[i + 1])) << 16u;
		}

		constexpr uint32_t GetTail(int tail) const
		{
			T tailValue = buffer[size - 1];
			if (tail == 0)
				return uint8_t(tailValue);
			else
				return uint8_t(tailValue >> 8u);
		}
	};

	template <typename T>
	struct HeartMurmurHashHelper<T, hrt::enable_if_t<sizeof(T) == 4>>
	{
		const T* const buffer;
		const size_t size;

		constexpr HeartMurmurHashHelper(const T* const b, const size_t s) :
			buffer(b), size(s)
		{
		}

		constexpr uint32_t GetBlock32(int index) const
		{
			return buffer[index];
		}
	};

	template <typename T>
	struct HeartMurmurHashHelper<T, hrt::enable_if_t<sizeof(T) == 8>>
	{
		const T* const buffer;
		const size_t size;

		constexpr HeartMurmurHashHelper(const T* const b, const size_t s) :
			buffer(b), size(s)
		{
		}

		constexpr uint32_t GetBlock32(int index) const
		{
			const int i = index / 2;
			const int shift = index % 2 == 0 ? 0u : 32u;
			return uint32_t(buffer[i] >> shift);
		}
	};

	// Support everything else too. This requires reinterpret_cast and cannot be constexpr though.
	template <typename T>
	struct HeartMurmurHashHelper<T, hrt::enable_if_t<sizeof(T) != 1 && sizeof(T) != 2 && sizeof(T) != 4 && sizeof(T) != 8>>
	{
		const uint8_t* const buffer;
		const size_t size;

		HeartMurmurHashHelper(const T* const b, const size_t s) :
			buffer(reinterpret_cast<const uint8_t* const>(b)), size(s * sizeof(T))
		{
		}

		uint32_t GetBlock32(int index) const
		{
			int i = index * 4;
			return uint32_t(buffer[i + 0]) |
				   uint32_t(buffer[i + 1]) << 8u |
				   uint32_t(buffer[i + 2]) << 16u |
				   uint32_t(buffer[i + 3]) << 24u;
		}

		uint32_t GetTail(int tail) const
		{
			const int tailSize = size % 4;
			return uint32_t(std::bit_cast<uint8_t>(buffer[size - tailSize + tail]));
		}
	};
}

constexpr static uint32_t HeartMurmurHash3DefaultSeed = 0x7C44A16D;

template <typename T>
constexpr uint32_t HeartMurmurHash3(const T* const data, const size_t count, const uint32_t seed = HeartMurmurHash3DefaultSeed)
{
	heart_priv::HeartMurmurHashHelper<T> helper(data, count);

	uint32_t h1 = seed;
	const int len = int(count * sizeof(T));
	const int nblocks = len / 4;

	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;

	// Body
	for (int i = 0; i < nblocks; ++i)
	{
		uint32_t k1 = helper.GetBlock32(i);

		k1 *= c1;
		k1 = std::rotl(k1, 15);
		k1 *= c2;

		h1 ^= k1;
		h1 = std::rotl(h1, 13);
		h1 = h1 * 5 + 0xe6546b64;
	}

	// Tail
	if constexpr (sizeof(T) < 4)
	{
		uint32_t k1 = 0;
		switch (len & 3)
		{
		case 3:
			k1 ^= helper.GetTail(2) << 16;
			[[fallthrough]];
		case 2:
			k1 ^= helper.GetTail(1) << 8;
			[[fallthrough]];
		case 1:
			k1 ^= helper.GetTail(0);
			k1 *= c1;
			k1 = std::rotl(k1, 15);
			k1 *= c2;
			h1 ^= k1;
		}
	}

	h1 ^= len;
	h1 ^= h1 >> 16;
	h1 *= 0x85ebca6b;
	h1 ^= h1 >> 13;
	h1 *= 0xc2b2ae35;
	h1 ^= h1 >> 16;

	return h1;
}

constexpr uint32_t HeartMurmurHash3(std::string_view string, uint32_t seed = HeartMurmurHash3DefaultSeed)
{
	return HeartMurmurHash3(string.data(), string.size(), seed);
}
