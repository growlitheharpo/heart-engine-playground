#pragma once

#define DECLARE_CANONICAL_COMPARISON_OPERATORS(type_name)                                                      \
		inline bool operator!= (const type_name& o) const { return !operator==(o); }                           \
		inline bool operator<= (const type_name& o) const { return operator<(o) || operator==(o); }            \
		inline bool operator>  (const type_name& o) const { return !operator<(o) && !operator==(o); }          \
		inline bool operator>= (const type_name& o) const { return !operator<(o); }

#define DECLARE_CANONICAL_COMPARISON_OPERATORS_BY_VALUE(type_name)                                      \
		inline bool operator!= (type_name o) const { return !operator==(o); }                           \
		inline bool operator<= (type_name o) const { return operator<(o) || operator==(o); }            \
		inline bool operator>  (type_name o) const { return !operator<(o) && !operator==(o); }          \
		inline bool operator>= (type_name o) const { return !operator<(o); }

// Canonical addition requires implementing the += operator and a copy constructor
#define DECLARE_CANONICAL_ADDITION_OPERATORS(type_name, diff_type)                                                    \
		inline type_name operator+ (diff_type offset) const { type_name tmp(*this); return tmp.operator+=(offset); }  \
		inline type_name& operator++ () { return operator+=(1); }                                                     \
		inline type_name operator++ (int) { type_name tmp(*this); this->operator++(); return tmp; }

// Canonical addition requires implementing the += operator and a copy constructor
#define DECLARE_CANONICAL_ADDITION_OPERATORS_NOPOST(type_name, diff_type)                                             \
		inline type_name operator+ (diff_type offset) const { type_name tmp(*this); return tmp.operator+=(offset); }  \
		inline type_name& operator++ () { return operator+=(1); }                                                     \
		inline type_name operator++ (int) = delete; // NO postfix!

// Canonical subtration requires implementing the -= operator and a copy constructor
#define DECLARE_CANONICAL_SUBTRACTION_OPERATORS(type_name, diff_type)                                                 \
		inline type_name operator- (diff_type offset) const { type_name tmp(*this); return tmp.operator-=(offset); }  \
		inline type_name& operator-- () { return operator-=(1); }                                                     \
		inline type_name operator-- (int) { type_name tmp(*this); this->operator--(); return tmp; }

// Canonical subtration requires implementing the -= operator and a copy constructor
#define DECLARE_CANONICAL_SUBTRACTION_OPERATORS_NOPOST(type_name, diff_type)                                          \
		inline type_name operator- (diff_type offset) const { type_name tmp(*this); return tmp.operator-=(offset); }  \
		inline type_name& operator-- () { return operator-=(1); }                                                     \
		inline type_name operator-- (int) = delete; // NO postfix!
