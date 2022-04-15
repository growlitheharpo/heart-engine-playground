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
#include <heart/types.h>

#include <gtest/gtest.h>

TEST(HeartTypes, BasicSizes)
{
	EXPECT_EQ(CHAR_BIT, 8);
	EXPECT_EQ(sizeof(uint8_t), 1);
	EXPECT_EQ(sizeof(byte_t), 1);
	EXPECT_EQ(sizeof(uint16_t), 2);
	EXPECT_EQ(sizeof(uint32_t), 4);
	EXPECT_EQ(sizeof(uint64_t), 8);
	EXPECT_GE(sizeof(size_t), 8);
	EXPECT_EQ(sizeof(void*), sizeof(size_t));
	EXPECT_EQ(sizeof(ptrdiff_t), sizeof(void*));
	EXPECT_EQ(sizeof(uintptr_t), sizeof(void*));
	EXPECT_EQ(sizeof(intptr_t), sizeof(void*));
}
