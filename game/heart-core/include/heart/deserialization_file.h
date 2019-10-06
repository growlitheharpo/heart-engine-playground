#pragma once

#include <heart/deserialization.h>
#include <heart/file.h>

#include <heart/debug/assert.h>

template <typename OutType>
bool HeartDeserializeObjectFromFile(OutType& outObject, const char* filename)
{
	// 4 kilobytes should be enough for just json... right?
	uint8_t filebuffer[4096] = {};

	HeartFile file;
	uint64_t size;
	if (!HeartOpenFile(file, filename, HeartOpenFileMode::ReadExisting))
		return false;

	if (!HeartGetFileSize(file, size))
		return false;

	if (!HEART_CHECK(size < sizeof(filebuffer), "Bump buffer size in DeserializeObjectFromFile or use dynamic alloc!"))
		return false;

	if (!HeartReadFile(file, filebuffer, size))
		return false;

	rapidjson::Document jsonDoc;
	jsonDoc.Parse((char*)(filebuffer));

	if (jsonDoc.HasParseError())
		return false;

	return HeartDeserializeObject(outObject, jsonDoc);
}
