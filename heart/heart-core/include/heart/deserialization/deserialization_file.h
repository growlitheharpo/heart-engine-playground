#pragma once

#include <heart/allocator.h>
#include <heart/deserialization/deserialization.h>
#include <heart/file.h>

#include <heart/debug/assert.h>

template <typename OutType, typename ByteAllocator = HeartDefaultTypedAllocator<uint8_t>>
bool HeartDeserializeObjectFromFile(OutType& outObject, const char* filename)
{
	ByteAllocator alloc;

	HeartFile file;
	uint64_t fileSize;
	if (!HeartOpenFile(file, filename, HeartOpenFileMode::ReadExisting))
		return false;

	if (!HeartGetFileSize(file, fileSize))
		return false;

	size_t bufferSize = size_t(fileSize) + 1;

	uint8_t* filebuffer = alloc.allocate(bufferSize);
	filebuffer[bufferSize - 1] = 0;

	if (!HeartReadFile(file, filebuffer, bufferSize, fileSize))
		return false;

	rapidjson::Document jsonDoc;
	jsonDoc.Parse((char*)(filebuffer));

	if (jsonDoc.HasParseError())
		return false;

	bool result = HeartDeserializeObject(outObject, jsonDoc);
	alloc.deallocate(filebuffer);

	return result;
}
