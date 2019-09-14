#include <heart/debug/assert.h>

#include <heart/stl/string.h>

#include <string.h>
#include <stdio.h>
#include <inttypes.h>

namespace heart_priv
{
	size_t HeartDebugStringify::Append(const char* str)
	{
		size_t origSize = target_size_;

		if (target_size_ < target_capacity_)
		{
			strcpy_s(target_ + target_size_, target_capacity_ - target_size_, str);
			target_size_ = strlen(target_); // TODO: write a strcopy that returns bytes written
		}

		size_t written = target_size_ - origSize;
		return written;
	}

	void HeartDebugStringify::AppendNextName()
	{
		if (target_size_ > 0)
			Append("\n");

		if (macroified_names_ == nullptr)
			return;

		const char* name = macroified_names_;
		if (*name == 0)
			return;

		size_t nameLength;
		if (*name != '"')
		{
			nameLength = Append(name);
			Append(" = ");
		}
		else
		{
			nameLength = strlen(name);
		}

		macroified_names_ = name + nameLength + 1;
	}
}

size_t WriteDebugValue(char* tgt, size_t len, bool value)
{
	int w = sprintf_s(tgt, len, "%s", value ? "true" : "false");
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, const char* value)
{
	int w = sprintf_s(tgt, len, "\"%s\"", value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, int8_t value)
{
	int w = sprintf_s(tgt, len, "%" PRId8, value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, uint8_t value)
{
	int w = sprintf_s(tgt, len, "%" PRIu8, value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, int16_t value)
{
	int w = sprintf_s(tgt, len, "%" PRId16, value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, uint16_t value)
{
	int w = sprintf_s(tgt, len, "%" PRIu16, value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, int32_t value)
{
	int w = sprintf_s(tgt, len, "%" PRId32, value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, uint32_t value)
{
	int w = sprintf_s(tgt, len, "%" PRIu32, value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, int64_t value)
{
	int w = sprintf_s(tgt, len, "%" PRId64, value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, uint64_t value)
{
	int w = sprintf_s(tgt, len, "%" PRIu64, value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, float value)
{
	int w = sprintf_s(tgt, len, "%f", value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, double value)
{
	int w = sprintf_s(tgt, len, "%f", value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, const hrt::string& value)
{
	int w = sprintf_s(tgt, len, "\"%s\"", value.c_str());
	return w < 0 ? 0 : size_t(w);
}
