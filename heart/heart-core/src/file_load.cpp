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
#include "heart/util/file_load.h"

hrt::vector<uint8_t> HeartUtilLoadExistingFile(const char* filename)
{
	hrt::vector<uint8_t> result;

	HeartFile file;
	if (!HeartOpenFile(file, filename, HeartOpenFileMode::ReadExisting))
		return result;

	uint64_t size;
	if (!HeartGetFileSize(file, size))
		return result;

	result.resize(size_t(size));
	if (!HeartReadFile(file, result.data(), result.size(), size_t(size)))
		result.clear();

	HeartCloseFile(file);
	return result;
}
