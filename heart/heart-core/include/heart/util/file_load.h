#pragma once

#include <heart/file.h>
#include <heart/stl/vector.h>


// Utility function - opens the given file using ReadExisting and
//  loads it into a hrt::vector before closing the file.
hrt::vector<uint8_t> HeartUtilLoadExistingFile(const char* filename);
