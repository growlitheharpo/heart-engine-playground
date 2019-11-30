#pragma once

#include <heart/deserialization.h>
#include <heart/allocator.h>
#include <heart/file.h>

#include <heart/debug/assert.h>

template <typename OutType, typename Allocator = HeartDefaultAllocator>
bool HeartDeserializeObjectFromFile(OutType& outObject, const char* filename)
{
	Allocator alloc;

	HeartFile file;
	uint64_t fileSize;
	if (!HeartOpenFile(file, filename, HeartOpenFileMode::ReadExisting))
		return false;

	if (!HeartGetFileSize(file, fileSize))
		return false;

	size_t bufferSize = size_t(fileSize) + 1;

	uint8_t* filebuffer = (uint8_t*)alloc.Allocate(bufferSize);
	filebuffer[bufferSize - 1] = 0;

	if (!HeartReadFile(file, filebuffer, bufferSize, fileSize))
		return false;

	rapidjson::Document jsonDoc;
	jsonDoc.Parse((char*)(filebuffer));

	if (jsonDoc.HasParseError())
		return false;

	bool result = HeartDeserializeObject(outObject, jsonDoc);
	alloc.Deallocate(filebuffer, bufferSize);

	return result;
}
