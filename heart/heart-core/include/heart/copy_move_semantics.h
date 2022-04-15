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

#define DISABLE_COPY_AND_MOVE_SEMANTICS(type_name)   \
	type_name(const type_name&) = delete;            \
	type_name(type_name&&) = delete;                 \
	type_name& operator=(const type_name&) = delete; \
	type_name& operator=(type_name&&) = delete

#define USE_DEFAULT_MOVE_SEMANTICS(type_name) \
	type_name(type_name&&) = default;         \
	type_name& operator=(type_name&&) = default

#define DISABLE_MOVE_SEMANTICS(type_name) \
	type_name(type_name&&) = delete;      \
	type_name& operator=(type_name&&) = delete

#define USE_DEFAULT_COPY_SEMANTICS(type_name) \
	type_name(const type_name&) = default;    \
	type_name& operator=(const type_name&) = default

#define DISABLE_COPY_SEMANTICS(type_name) \
	type_name(const type_name&) = delete; \
	type_name& operator=(const type_name&) = delete
