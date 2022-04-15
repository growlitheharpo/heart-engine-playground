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

#define DECLARE_STANDARD_TYPEDEFS(input_type) \
	typedef input_type value_type;            \
	typedef input_type* pointer;              \
	typedef const input_type* const_pointer;  \
	typedef input_type& reference;            \
	typedef const input_type& const_reference;

#define USING_STANDARD_TYPEDEFS(input_type)                   \
	using value_type = typename input_type::value_type;       \
	using pointer = typename input_type::pointer;             \
	using const_pointer = typename input_type::const_pointer; \
	using reference = typename input_type::reference;         \
	using const_reference = typename input_type::const_reference;

#define REF_AND_CONST_REF(func_signature, implementation) \
	reference func_signature implementation;              \
	const_reference func_signature const implementation

#define POINTER_AND_CONST_POINTER(func_signature, implementation) \
	pointer func_signature implementation;                        \
	const_pointer func_signature const implementation
