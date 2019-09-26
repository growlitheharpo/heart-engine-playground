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

bool HeartGetFileOffset(HeartFile& file, uint64_t& outOffset);

bool HeartSetFileOffset(HeartFile& file, uint64_t offset, uint64_t* newOffset = nullptr, HeartSetOffsetMode mode = HeartSetOffsetMode::Beginning);

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
	uintptr_t native_handle_ = 0;

	~HeartFile()
	{
		HeartCloseFile(*this);
	}
};