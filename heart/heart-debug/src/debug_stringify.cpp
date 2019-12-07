#include <heart/debug/assert.h>

#include <heart/stl/string.h>

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

size_t WriteDebugValue(char* tgt, size_t len, bool value)
{
	int w = sprintf_s(tgt, len, "%s\n", value ? "true" : "false");
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, const char* value)
{
	int w = sprintf_s(tgt, len, "\"%s\"\n", value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, int8_t value)
{
	int w = sprintf_s(tgt, len, "%" PRId8 "\n", value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, uint8_t value)
{
	int w = sprintf_s(tgt, len, "%" PRIu8 "\n", value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, int16_t value)
{
	int w = sprintf_s(tgt, len, "%" PRId16 "\n", value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, uint16_t value)
{
	int w = sprintf_s(tgt, len, "%" PRIu16 "\n", value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, int32_t value)
{
	int w = sprintf_s(tgt, len, "%" PRId32 "\n", value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, uint32_t value)
{
	int w = sprintf_s(tgt, len, "%" PRIu32 "\n", value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, int64_t value)
{
	int w = sprintf_s(tgt, len, "%" PRId64 "\n", value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, uint64_t value)
{
	int w = sprintf_s(tgt, len, "%" PRIu64 "\n", value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, float value)
{
	int w = sprintf_s(tgt, len, "%f\n", value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, double value)
{
	int w = sprintf_s(tgt, len, "%f\n", value);
	return w < 0 ? 0 : size_t(w);
}

size_t WriteDebugValue(char* tgt, size_t len, const hrt::string& value)
{
	int w = sprintf_s(tgt, len, "\"%s\"\n", value.c_str());
	return w < 0 ? 0 : size_t(w);
}
