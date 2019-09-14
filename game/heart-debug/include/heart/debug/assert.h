#pragma once

#include <heart/config.h>
#include <heart/types.h>
#include <heart/debug/message_box.h>

#define HEART_EXPAND(...) __VA_ARGS__
#define _HEART_STRINGIFY_10_ARGS(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, ...) \
	#A0 "\0" #A1 "\0" #A2 "\0" #A3 "\0" #A4 "\0" #A5 "\0" #A6 "\0" #A7 "\0" #A8 "\0" #A9 "\0" #__VA_ARGS__

// pad out with extra commas to ensure macro substitution succeeds no matter how many args we get
#define HEART_STRINGIFY_NAMES(...) HEART_EXPAND(_HEART_STRINGIFY_10_ARGS(__VA_ARGS__\0\0,,,,,,,,,,))

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

namespace heart_priv
{
	struct HeartDebugStringify
	{
		char* target_ = nullptr;
		size_t target_size_ = 0;
		size_t target_capacity_ = 0;
		const char* macroified_names_ = nullptr;

		HeartDebugStringify(char* tgt, size_t tgtLen, const char* names = nullptr) : target_(tgt), target_capacity_(tgtLen), macroified_names_(names)
		{
			if (target_capacity_ > 0 && target_)
				*target_ = 0;
		}

		size_t Append(const char* str);

		void AppendNextName();

		template <typename T>
		void AppendValue(const T& val)
		{
			AppendNextName();
			size_t s = WriteDebugValue(target_ + target_size_, target_capacity_ - target_size_, val);
			target_size_ += s;
		}

		template <typename... Args>
		const char* operator()(const Args... vals)
		{
			if (target_capacity_ > 0 && target_ && *target_ == 0)
			{
				(AppendValue(vals), ...);
			}
			return target_;
		}
	};
}

#define HEART_TO_DEBUG_STR(tgt, tgtLen, ...) heart_priv::HeartDebugStringify(tgt, tgtLen, HEART_STRINGIFY_NAMES(__VA_ARGS__))(__VA_ARGS__)

#define HEART_ERROR(title, msg, ...) ([&](...) { \
	static bool s_ignored = false;          \
	char details[2048];                     \
	return DisplayAssertError(title, msg, HEART_TO_DEBUG_STR(details, 2048, __VA_ARGS__), __FILE__, __LINE__, &s_ignored); }() && (__debugbreak(), false))

#if HEART_STRICT_PERF

#define HEART_CHECK(expr, ...) (!!(expr))
#define HEART_ASSERT(...) ((void)0)

#else

#define HEART_CHECK(expr, ...) ((!!(expr)) || HEART_ERROR("HEART_CHECK FAILED!", "HEART_CHECK: " #expr, __VA_ARGS__))
#define HEART_ASSERT(expr, ...) (void)((!!(expr)) || HEART_ERROR("HEART_ASSERT FAILED!", "HEART_ASSERT: " #expr, __VA_ARGS__))

#endif
