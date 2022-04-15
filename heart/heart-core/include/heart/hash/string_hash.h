#pragma once

#include <heart/hash/murmur.h>

#include <heart/config.h>

#include <array>

constexpr static uint32_t HeartInvalidStringHash = -1;

template <size_t N = 96>
class HeartStringHash;
class HeartConstStringHash;

#ifdef HEARTPRIV_WHEN_INCLUDE_DEBUG_STRINGS
#error Internal HEARTPRIV macro redefined by user, this is illegal.
#endif

#if HEART_INCLUDE_DEBUG_STRINGS
#define HEARTPRIV_WHEN_INCLUDE_DEBUG_STRINGS(...) __VA_ARGS__
#else
#define HEARTPRIV_WHEN_INCLUDE_DEBUG_STRINGS(...)
#endif

/// <summary>
/// HeartConstStringHash provides a string hash for cases where the debug string is either
/// never used or is known to either be a compile-time const char* or a runtime value that
/// will outlive the hash. When debug strings are enabled, it simply holds a raw pointer to
/// the string and does not do anything to keep it alive.
/// In configurations where debug strings are not enabled, HeartConstStringHash and HeartStringHash
/// have exactly identical behavior, size, and interfaces.
/// </summary>
class HeartConstStringHash
{
public:
	constexpr HeartConstStringHash() :
		m_value(HeartInvalidStringHash) HEARTPRIV_WHEN_INCLUDE_DEBUG_STRINGS(, m_string())
	{
	}

	constexpr HeartConstStringHash(std::string_view str) :
		m_value(HeartMurmurHash3(str)) HEARTPRIV_WHEN_INCLUDE_DEBUG_STRINGS(, m_string(str))
	{
	}

	constexpr uint32_t Value() const
	{
		return m_value;
	}

	constexpr bool Valid() const
	{
		return m_value != HeartInvalidStringHash;
	}

	constexpr HeartConstStringHash(const HeartConstStringHash&) noexcept = default;

	constexpr HeartConstStringHash(HeartConstStringHash&&) noexcept = default;

	constexpr HeartConstStringHash& operator=(const HeartConstStringHash&) noexcept = default;

	constexpr HeartConstStringHash& operator=(HeartConstStringHash&&) noexcept = default;

	template <size_t N>
	constexpr HeartConstStringHash(const HeartStringHash<N>& o) noexcept;

	template <size_t N>
	constexpr HeartConstStringHash(HeartStringHash<N>&& o) noexcept;

	HEARTPRIV_WHEN_INCLUDE_DEBUG_STRINGS(
		constexpr std::string_view DebugString() const {
			return m_string;
		}

		constexpr const char* DebugCStringUnsafe() const {
			return m_string.data();
		})

private:
	uint32_t m_value;

	HEARTPRIV_WHEN_INCLUDE_DEBUG_STRINGS(std::string_view m_string);
};

/// <summary>
/// HeartStringHash provides a string hash for cases where the debug string needs to be preserved
/// by the hash itself because it is inspected and its source is not known to be constant or kept
/// alive by the initializer. When debug strings are enabled, it copies the source string into a
/// debug storage buffer.
/// In configurations where debug strings are not enabled, HeartConstStringHash and HeartStringHash
/// have exactly identical behavior, size, and interfaces.
/// </summary>
template <size_t N>
class HeartStringHash
{
public:
	constexpr HeartStringHash() :
		m_value(HeartInvalidStringHash) HEARTPRIV_WHEN_INCLUDE_DEBUG_STRINGS(, m_string(), m_stringSize(0))
	{
	}

	constexpr HeartStringHash(std::string_view v) :
		m_value(HeartMurmurHash3(v)) HEARTPRIV_WHEN_INCLUDE_DEBUG_STRINGS(, m_string())
	{
#if HEART_INCLUDE_DEBUG_STRINGS
		m_stringSize = std::min(v.size(), m_string.size());
		for (size_t i = 0; i < m_stringSize; ++i)
		{
			m_string[i] = v[i];
		}
#endif
	}

	constexpr uint32_t Value() const
	{
		return m_value;
	}

	constexpr bool Valid() const
	{
		return m_value != HeartInvalidStringHash;
	}

	constexpr HeartStringHash(const HeartStringHash&) noexcept = default;

	constexpr HeartStringHash(HeartStringHash&&) noexcept = default;

	constexpr HeartStringHash& operator=(const HeartStringHash&) noexcept = default;

	constexpr HeartStringHash& operator=(HeartStringHash&&) noexcept = default;

	constexpr HeartStringHash(const HeartConstStringHash& o);

	constexpr HeartStringHash(HeartConstStringHash&& o);

	HEARTPRIV_WHEN_INCLUDE_DEBUG_STRINGS(
		constexpr std::string_view DebugString() const {
			return std::string_view(m_string.data(), m_stringSize);
		}

		constexpr const char* DebugCStringUnsafe() const {
			return m_string.data();
		})

private:
	uint32_t m_value = HeartInvalidStringHash;

#if HEART_INCLUDE_DEBUG_STRINGS
	std::array<char, N> m_string;
	size_t m_stringSize;
#endif
};

template <size_t N>
inline constexpr HeartConstStringHash::HeartConstStringHash(const HeartStringHash<N>& o) noexcept :
	m_value(o.Value()) HEARTPRIV_WHEN_INCLUDE_DEBUG_STRINGS(, m_string(o.DebugString()))
{
}

template <size_t N>
inline constexpr HeartConstStringHash::HeartConstStringHash(HeartStringHash<N>&& o) noexcept :
	m_value(o.Value()) HEARTPRIV_WHEN_INCLUDE_DEBUG_STRINGS(, m_string(o.DebugString()))
{
}

template <size_t N>
inline constexpr HeartStringHash<N>::HeartStringHash(const HeartConstStringHash& o) :
	m_value(o.Value()) HEARTPRIV_WHEN_INCLUDE_DEBUG_STRINGS(, m_string())
{
#if HEART_INCLUDE_DEBUG_STRINGS
	m_stringSize = std::min(o.DebugString().size(), m_string.size());
	for (size_t i = 0; i < m_stringSize; ++i)
	{
		m_string[i] = o.DebugString()[i];
	}
#endif
}

template <size_t N>
inline constexpr HeartStringHash<N>::HeartStringHash(HeartConstStringHash&& o) :
	m_value(o.Value()) HEARTPRIV_WHEN_INCLUDE_DEBUG_STRINGS(, m_string())
{
#if HEART_INCLUDE_DEBUG_STRINGS
	m_stringSize = std::min(o.DebugString().size(), m_string.size());
	for (size_t i = 0; i < m_stringSize; ++i)
	{
		m_string[i] = o.DebugString()[i];
	}
#endif
}

#undef HEARTPRIV_WHEN_INCLUDE_DEBUG_STRINGS
