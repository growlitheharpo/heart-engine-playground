/* Copyright (C) 2022 James Keats
*
* This file is part of Heart, a collection of game engine technologies.
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
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
