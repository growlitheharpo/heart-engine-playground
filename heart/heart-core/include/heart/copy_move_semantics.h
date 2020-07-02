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
