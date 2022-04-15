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
#include "heart/sleep.h"

#include "priv/SlimWin32.h"

void HeartYield()
{
	Sleep(0);
}

void HeartSleep(uint32_t milliseconds)
{
	Sleep(milliseconds);
}
