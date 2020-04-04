#include "heart/file.h"

#include "heart/debug/assert.h"

#include <stdio.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <WinBase.h>
#include <fileapi.h>

static wchar_t s_file_root[MAX_PATH];

static const char* CwdToken = "{%cwd}";

void HeartSetRoot(const char* root)
{
	if (root == nullptr)
		return;

	auto cwdToken = strstr(root, CwdToken);
	if (cwdToken != nullptr)
	{
		HEART_ASSERT(cwdToken == root, "Cannot insert CWD in the middle of the root path!");
		size_t written = GetCurrentDirectory(MAX_PATH, s_file_root);
		swprintf_s(s_file_root + written, MAX_PATH - written, L"%S", root + strlen(CwdToken));
	}
	else
	{
		swprintf_s(s_file_root, L"%S", root);
	}
}

bool HeartOpenFile(HeartFile& outFile, const char* path, HeartOpenFileMode mode)
{
	outFile = {};

	wchar_t filePath[MAX_PATH];
	int written = swprintf_s(filePath, L"%s", s_file_root);
	if (written < 0)
		return false;

	written = swprintf_s(filePath + written, MAX_PATH - written, L"%S", path);
	if (written <= 0)
		return false;

	DWORD access = 0, creation = 0;
	switch (mode)
	{
	case HeartOpenFileMode::ReadExisting:
		access = GENERIC_READ;
		creation = OPEN_EXISTING;
		break;
	case HeartOpenFileMode::ReadWriteExisting:
		access = GENERIC_READ | GENERIC_WRITE;
		creation = OPEN_EXISTING;
		break;
	case HeartOpenFileMode::ReadWriteCreate:
		access = GENERIC_READ | GENERIC_WRITE;
		creation = OPEN_ALWAYS;
		break;
	case HeartOpenFileMode::Write:
		access = GENERIC_WRITE;
		creation = OPEN_ALWAYS;
		break;
	case HeartOpenFileMode::WriteCreateAlways:
		access = GENERIC_WRITE;
		creation = CREATE_NEW;
		break;
	case HeartOpenFileMode::WriteExisting:
		access = GENERIC_WRITE;
		creation = OPEN_EXISTING;
		break;
	case HeartOpenFileMode::WriteTruncateExisting:
		access = GENERIC_WRITE;
		creation = TRUNCATE_EXISTING;
		break;
	}

	DWORD sharing = FILE_SHARE_READ;

	HANDLE result = CreateFile(filePath, access, sharing, NULL, creation, FILE_ATTRIBUTE_NORMAL, NULL);
	if (result == INVALID_HANDLE_VALUE)
		return false;

	outFile.native_handle_ = uintptr_t(result);
	return true;
}

bool HeartCloseFile(HeartFile& file)
{
	if (file.native_handle_ == 0)
		return true;

	auto result = bool(CloseHandle(HANDLE(file.native_handle_)));
	file.native_handle_ = 0;
	return result;
}

bool HeartGetFileSize(HeartFile& file, uint64_t& outSize)
{
	outSize = 0;

	if (file.native_handle_ == 0)
		return false;

	LARGE_INTEGER size = {};
	BOOL result = GetFileSizeEx(HANDLE(file.native_handle_), &size);
	if (result == FALSE)
		return false;

	outSize = size.QuadPart;
	return true;
}

bool HeartGetFileOffset(HeartFile& file, uint64_t& outOffset)
{
	outOffset = 0;

	if (file.native_handle_ == 0)
		return false;

	LARGE_INTEGER inMove = {};
	LARGE_INTEGER outMove = {};
	BOOL result = SetFilePointerEx(HANDLE(file.native_handle_), inMove, &outMove, FILE_CURRENT);
	if (result == 0)
		return false;

	outOffset = outMove.QuadPart;
	return true;
}

bool HeartSetFileOffset(HeartFile& file, uint64_t offset, uint64_t* newOffset, HeartSetOffsetMode mode)
{
	if (file.native_handle_ == 0)
		return false;

	uint64_t localNewOffset;
	if (newOffset == nullptr)
		newOffset = &localNewOffset;

	LARGE_INTEGER inMove = {};
	inMove.QuadPart = offset;

	LARGE_INTEGER outMove = {};

	DWORD moveMode = 0;
	switch (mode)
	{
	case HeartSetOffsetMode::Current: moveMode = FILE_CURRENT; break;
	case HeartSetOffsetMode::Beginning: moveMode = FILE_BEGIN; break;
	case HeartSetOffsetMode::End: moveMode = FILE_END; break;
	}

	BOOL result = SetFilePointerEx(HANDLE(file.native_handle_), inMove, &outMove, moveMode);
	if (result == FALSE)
		return false;

	*newOffset = outMove.QuadPart;
	return true;
}

bool HeartReadFile(HeartFile& file, byte_t* buffer, size_t size, size_t bytesToRead, size_t* bytesRead)
{
	if (file.native_handle_ == 0)
		return false;

	if (!HEART_CHECK(size >= bytesToRead, "Trying to read into a buffer that's not large enough!"))
		return false;

	if (!HEART_CHECK(bytesToRead < MAXDWORD, "Cannot read more than MAXDWORD at once!", MAXDWORD, bytesToRead))
		return false;

	uint64_t localBytesRead;
	if (bytesRead == nullptr)
		bytesRead = &localBytesRead;

	DWORD dwBytesRead;
	BOOL result = ReadFile(HANDLE(file.native_handle_), buffer, DWORD(bytesToRead), &dwBytesRead, NULL);
	if (result == FALSE)
		return false;

	*bytesRead = size_t(dwBytesRead);
	return true;
}

bool HeartWriteFile(HeartFile& file, byte_t* buffer, size_t bytesToWrite, size_t* bytesWritten)
{
	if (file.native_handle_ == 0)
		return false;

	if (!HEART_CHECK(bytesToWrite < MAXDWORD, "Cannot write more than MAXDWORD at once!", MAXDWORD, bytesToWrite))
		return false;

	uint64_t localBytesWritten;
	if (bytesWritten == nullptr)
		bytesWritten = &localBytesWritten;

	DWORD dwBytesWritten;
	BOOL result = WriteFile(HANDLE(file.native_handle_), buffer, DWORD(bytesToWrite), &dwBytesWritten, NULL);
	if (result == FALSE)
		return false;

	*bytesWritten = size_t(dwBytesWritten);
	return true;
}
