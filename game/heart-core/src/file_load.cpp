#include <heart/util/file_load.h>

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
