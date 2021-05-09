#pragma once

#include <heart/stl/type_traits/constants.h>

namespace hrt
{

	// most aligned type on x64 MSVC
	using max_align_t = double;

	// determine alignment of T
	template <class T>
	struct alignment_of : integral_constant<size_t, alignof(T)>
	{
	};

	template <class T>
	constexpr size_t alignment_of_v = alignof(T);

	// The following is just directly copied from MSVC's <type_traits>

	// STRUCT TEMPLATE aligned_storage
	// union with size len bytes and alignment of T
	template <class T, size_t len>
	union _Align_type {
		T val;
		char padding[len];
	};

	template <size_t _Len, size_t _Align, class _Ty, bool _Ok>
	struct _Aligned;

	template <size_t _Len, size_t _Align, class _Ty>
	struct _Aligned<_Len, _Align, _Ty, true>
	{
		using type = _Align_type<_Ty, _Len>;
	};

	template <size_t _Len, size_t _Align>
	struct _Aligned<_Len, _Align, double, false>
	{
		struct type
		{
			alignas(_Align) char _Space[_Len];
		};
	};

	template <size_t _Len, size_t _Align>
	struct _Aligned<_Len, _Align, int, false>
	{
		using _Next = double;
		static constexpr bool _Fits = _Align <= alignof(_Next);
		using type = typename _Aligned<_Len, _Align, _Next, _Fits>::type;
	};

	template <size_t _Len, size_t _Align>
	struct _Aligned<_Len, _Align, short, false>
	{
		using _Next = int;
		static constexpr bool _Fits = _Align <= alignof(_Next);
		using type = typename _Aligned<_Len, _Align, _Next, _Fits>::type;
	};

	template <size_t _Len, size_t _Align>
	struct _Aligned<_Len, _Align, char, false>
	{
		using _Next = short;
		static constexpr bool _Fits = _Align <= alignof(_Next);
		using type = typename _Aligned<_Len, _Align, _Next, _Fits>::type;
	};

	template <size_t _Len, size_t _Align = alignof(max_align_t)>
	struct aligned_storage
	{
		// define type with size _Len and alignment _Align
		using _Next = char;
		static constexpr bool _Fits = _Align <= alignof(_Next);
		using type = typename _Aligned<_Len, _Align, _Next, _Fits>::type;
	};

	template <size_t _Len, size_t _Align = alignof(max_align_t)>
	using aligned_storage_t = typename aligned_storage<_Len, _Align>::type;
}
