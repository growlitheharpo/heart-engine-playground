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

#include <heart/types.h>

struct HeartFile;

enum class HeartOpenFileMode
{
	ReadExisting,
	ReadWriteExisting,
	ReadWriteCreate,
	Write,
	WriteCreateAlways,
	WriteExisting,
	WriteTruncateExisting,
};

enum class HeartSetOffsetMode
{
	Current,
	Beginning,
	End,
};

void HeartSetRoot(const char* root);

bool HeartOpenFile(HeartFile& outFile, const char* path, HeartOpenFileMode mode);

bool HeartCloseFile(HeartFile& file);

bool HeartGetFileSize(HeartFile& file, uint64_t& outSize);

bool HeartGetFileSize(const char* path, uint64_t& outSize);

bool HeartGetFileOffset(HeartFile& file, uint64_t& outOffset);

bool HeartSetFileOffset(HeartFile& file, int64_t offset, uint64_t* newOffset = nullptr, HeartSetOffsetMode mode = HeartSetOffsetMode::Beginning);

bool HeartReadFile(HeartFile& file, byte_t* buffer, size_t size, size_t bytesToRead, size_t* bytesRead = nullptr);

template <size_t N>
bool HeartReadFile(HeartFile& file, byte_t (&buffer)[N], size_t bytesToRead, size_t* bytesRead = nullptr)
{
	return HeartReadFile(file, buffer, N, bytesToRead, bytesRead);
}

bool HeartWriteFile(HeartFile& file, byte_t* buffer, size_t size, size_t* bytesWritten = nullptr);

template <size_t N>
bool HeartWriteFile(HeartFile& file, byte_t (&buffer)[N], size_t* bytesWritten = nullptr)
{
	return HeartWriteFile(file, buffer, N, bytesWritten);
}

struct HeartFile
{
	uintptr_t nativeHandle = 0;

	explicit operator bool() const
	{
		return nativeHandle != 0;
	}

	~HeartFile()
	{
		HeartCloseFile(*this);
	}
};
