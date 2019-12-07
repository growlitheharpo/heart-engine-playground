#pragma once

#include <heart/config.h>
#include <heart/types.h>

#include <heart/debug/message_box.h>

size_t WriteDebugValue(char* tgt, size_t len, bool value);
size_t WriteDebugValue(char* tgt, size_t len, const char* value);
size_t WriteDebugValue(char* tgt, size_t len, int8_t value);
size_t WriteDebugValue(char* tgt, size_t len, uint8_t value);
size_t WriteDebugValue(char* tgt, size_t len, int16_t value);
size_t WriteDebugValue(char* tgt, size_t len, uint16_t value);
size_t WriteDebugValue(char* tgt, size_t len, int32_t value);
size_t WriteDebugValue(char* tgt, size_t len, uint32_t value);
size_t WriteDebugValue(char* tgt, size_t len, int64_t value);
size_t WriteDebugValue(char* tgt, size_t len, uint64_t value);
size_t WriteDebugValue(char* tgt, size_t len, float value);
size_t WriteDebugValue(char* tgt, size_t len, double value);

template <typename... Ts>
struct HeartAssertWriter
{
	char buffer[4096] = {};
	char* writer = buffer;

	template <typename T>
	void WriteValue(const T& val)
	{
		const char* const end = (buffer + sizeof(buffer));
		writer += WriteDebugValue(writer, end - writer, val);
	}

	HeartAssertWriter(Ts... args)
	{
		memset(buffer, 0, sizeof(buffer));
		(WriteValue(args), ...);
	}
};

#define HEART_ASSERT_FAILED(title, msg, ...)                                                                           \
	([&](...) {                                                                                                        \
		static bool s_ignored = false;                                                                                 \
		HeartAssertWriter r(__VA_ARGS__);                                                                              \
		return DisplayAssertError(title, msg, r.buffer, __FILE__, __LINE__, &s_ignored);                               \
	}() &&                                                                                                             \
		(__debugbreak(), false))

#if HEART_STRICT_PERF

#define HEART_ASSERT(expr, ...) ((void)0)

#define HEART_CHECK(expr, ...) (!!(expr))

#else

#define HEART_ASSERT(expr, ...)                                                                                        \
	((!!(expr)) || HEART_ASSERT_FAILED("HEART_ASSERT FAILED!", "HEART_ASERT: ", #expr, __VA_ARGS__))

#define HEART_CHECK(expr, ...)                                                                                         \
	((!!(expr)) || HEART_ASSERT_FAILED("HEART_CHECK FAILED!", "HEART_ASERT: ", #expr, __VA_ARGS__))

#endif
