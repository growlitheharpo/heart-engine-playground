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

#include <heart/file.h>
#include <heart/stl/vector.h>

// Utility function - opens the given file using ReadExisting and
//  loads it into a hrt::vector before closing the file.
hrt::vector<uint8_t> HeartUtilLoadExistingFile(const char* filename);
