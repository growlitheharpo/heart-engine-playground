#pragma once

#include <heart/stl/forward.h>

#include <heart/stl/type_traits/add_remove_ref_cv.h>
#include <heart/stl/type_traits/conditional.h>
#include <heart/stl/type_traits/convertible.h>
#include <heart/stl/type_traits/void.h>

namespace hrt
{
	// This one is just straight-up ripped from MSVC/Dinkumware

	/* FUNCTION TEMPLATE invoke */
	struct Invoker_pmf_object
	{
		/* INVOKE a pointer to member function on an object */
		template <class Decayed, class T1, class... Args>
		static constexpr auto call(Decayed pmf, T1&& arg1, Args&&... args2) noexcept(
			noexcept((forward<T1>(arg1).*pmf)(forward<Args>(args2)...)))
			-> decltype((forward<T1>(arg1).*pmf)(forward<Args>(args2)...))
		{
			/* INVOKE a pointer to member function on an object */
			return (forward<T1>(arg1).*pmf)(forward<Args>(args2)...);
		}
	};

	struct Invoker_pmf_refwrap
	{
		/* INVOKE a pointer to member function on a reference_wrapper */
		template <class Decayed, class T1, class... Args>
		static constexpr auto call(Decayed pmf, T1&& arg1, Args&&... args2) noexcept(
			noexcept((forward<T1>(arg1).get().*pmf)(forward<Args>(args2)...)))
			-> decltype((forward<T1>(arg1).get().*pmf)(
				forward<Args>(args2)...))
		{
			/* INVOKE a pointer to member function on a reference_wrapper */
			return (forward<T1>(arg1).get().*pmf)(forward<Args>(args2)...);
		}
	};

	struct Invoker_pmf_pointer
	{
		/* INVOKE a pointer to member function on a [smart] pointer */
		template <class Decayed, class T1, class... Args>
		static constexpr auto call(Decayed pmf, T1&& arg1, Args&&... args2) noexcept(
			noexcept(((*forward<T1>(arg1)).*pmf)(forward<Args>(args2)...)))
			-> decltype(((*forward<T1>(arg1)).*pmf)(
				forward<Args>(args2)...))
		{
			/* INVOKE a pointer to member function on a [smart] pointer */
			return ((*forward<T1>(arg1)).*pmf)(forward<Args>(args2)...);
		}
	};

	struct Invoker_pmd_object
	{
		/* INVOKE a pointer to member data on an object */
		template <class Decayed, class T1>
		static constexpr auto call(Decayed pmd, T1&& arg1) noexcept(noexcept(forward<T1>(arg1).*pmd))
			-> decltype(forward<T1>(arg1).*pmd)
		{
			/* INVOKE a pointer to member data on an object */
			return forward<T1>(arg1).*pmd;
		}
	};

	struct Invoker_pmd_refwrap
	{
		/* INVOKE a pointer to member data on a reference_wrapper */
		template <class Decayed, class T1>
		static constexpr auto call(Decayed pmd, T1&& arg1) noexcept(
			noexcept(forward<T1>(arg1).get().*pmd))
			-> decltype(
				forward<T1>(arg1).get().*pmd)
		{
			/* INVOKE a pointer to member data on a reference_wrapper */
			return forward<T1>(arg1).get().*pmd;
		}
	};

	struct Invoker_pmd_pointer
	{
		/* INVOKE a pointer to member data on a [smart] pointer */
		template <class Decayed, class T1>
		static constexpr auto call(Decayed pmd, T1&& arg1) noexcept(
			noexcept((*forward<T1>(arg1)).*pmd))
			-> decltype(
				(*forward<T1>(arg1)).*pmd)
		{
			/* INVOKE a pointer to member data on a [smart] pointer */
			return (*forward<T1>(arg1)).*pmd;
		}
	};

	struct Invoker_functor
	{
		/* INVOKE a function object */
		template <class Callable, class... Types>
		static constexpr auto call(Callable&& obj, Types&&... args) noexcept(
			noexcept(forward<Callable>(obj)(forward<Types>(args)...)))
			-> decltype(
				forward<Callable>(obj)(forward<Types>(args)...))
		{
			/* INVOKE a function object */
			return forward<Callable>(obj)(forward<Types>(args)...);
		}
	};

	template <class Callable, class T1, class Removed_cvref = remove_cvref_t<Callable>,
		bool Is_pmf = is_member_function_pointer_v<Removed_cvref>,
		bool Is_pmd = is_member_object_pointer_v<Removed_cvref>>
	struct Invoker1;

	template <class Callable, class T1, class Removed_cvref>
	struct Invoker1<Callable, T1, Removed_cvref, true, false> : conditional_t<is_base_of_v<typename Is_memfunptr<Removed_cvref>::Class_type, remove_reference_t<T1>>, Invoker_pmf_object, conditional_t<Is_specialization_v<Remove_cvref_t<T1>, reference_wrapper>, Invoker_pmf_refwrap, Invoker_pmf_pointer>>
	{
		/* pointer to member function */
	};

	template <class Callable, class T1, class Removed_cvref>
	struct Invoker1<Callable, T1, Removed_cvref, false, true> : conditional_t<is_base_of_v<typename Is_member_object_pointer<Removed_cvref>::Class_type, remove_reference_t<T1>>, Invoker_pmd_object, conditional_t<Is_specialization_v<Remove_cvref_t<T1>, reference_wrapper>, Invoker_pmd_refwrap, Invoker_pmd_pointer>>
	{
		/* pointer to member data */
	};

	template <class Callable, class T1, class Removed_cvref>
	struct Invoker1<Callable, T1, Removed_cvref, false, false> : Invoker_functor
	{
		/* function object */
	};

	template <class Callable, class... Types>
	struct Invoker;

	template <class Callable>
	struct Invoker<Callable> : Invoker_functor
	{
		/* zero arguments */
	};

	template <class Callable, class T1, class... Args>
	struct Invoker<Callable, T1, Args...> : Invoker1<Callable, T1>
	{
		/* one or more arguments */
	};

	template <class Callable, class... Types>
	constexpr auto invoke(Callable&& obj, Types&&... Args) noexcept(
		noexcept(Invoker<Callable,
			Types...>::call(forward<Callable>(obj), forward<Types>(Args)...)))
		-> decltype(Invoker<Callable,
			Types...>::call(forward<Callable>(obj), forward<Types>(Args)...))
	{
		/* INVOKE a callable object */
		return Invoker<Callable, Types...>::call(
			forward<Callable>(obj), forward<Types>(Args)...);
	}

	template <class Rx, bool = is_void_v<Rx>>
	struct Invoker_ret
	{
	};

	struct Unforced
	{
	};

	template <class Cv_void>
	struct Invoker_ret<Cv_void, true>
	{
		// selected for Rx being cv void
		template <class... Valtys>
		static void call(Valtys&&... Vals)
		{
			// INVOKE, "implicitly" converted to void
			invoke(forward<Valtys>(Vals)...);
		}
	};

	template <class Rx>
	struct Invoker_ret<Rx, false>
	{
		// selected for all Rx other than cv void and Unforced
		template <class... Valtys>
		static Rx call(Valtys&&... Vals)
		{
			// INVOKE, implicitly converted to Rx
			return invoke(forward<Valtys>(Vals)...);
		}
	};

	template <>
	struct Invoker_ret<Unforced, false>
	{
		// selected for Rx being Unforced
		template <class... Valtys>
		static auto call(Valtys&&... Vals)
			-> decltype(invoke(forward<Valtys>(Vals)...))
		{
			// INVOKE, unchanged
			return invoke(forward<Valtys>(Vals)...);
		}
	};

	// TYPE TRAITS FOR invoke()

	template <class Void, class... Types>
	struct Invoke_traits
	{
		// selected when Callable isn't callable with Args
		using Is_invocable = false_type;
		using Is_nothrow_invocable = false_type;
		template <class Rx>
		using Is_invocable_r = false_type;
		template <class Rx>
		using Is_nothrow_invocable_r = false_type;
	};

	template <class... Types>
	struct Invoke_traits<void_t<decltype(invoke(declval<Types>()...))>,
		Types...>
	{
		// selected when Callable is callable with Args
		using type = decltype(invoke(declval<Types>()...));
		using Is_invocable = true_type;
		using Is_nothrow_invocable = bool_constant<noexcept(invoke(declval<Types>()...))>;
		template <class Rx>
		using Is_invocable_r = bool_constant<disjunction_v<is_void<Rx>, is_convertible<type, Rx>>>;
		template <class Rx>
		using Is_nothrow_invocable_r = bool_constant<
			conjunction_v<Is_nothrow_invocable, disjunction<is_void<Rx>, Is_nothrow_convertible<type, Rx>>>>;
	};

	template <class Callable, class... Args>
	using Invoke_result_t = typename Invoke_traits<void, Callable, Args...>::type;

	template <class Rx, class Callable, class... Args>
	using Is_invocable_r_ = typename Invoke_traits<void, Callable, Args...>::template Is_invocable_r<Rx>;

	template <class Rx, class Callable, class... Args>
	struct Is_invocable_r : Is_invocable_r_<Rx, Callable, Args...>
	{
		// determines whether Callable is callable with Args and return type Rx
	};

	// STRUCT TEMPLATE invoke_result
	template <class Callable, class... Args>
	struct invoke_result : Invoke_traits<void, Callable, Args...>
	{
		// determine the result type of invoking Callable with Args
	};

	template <class Callable, class... Args>
	using invoke_result_t = typename Invoke_traits<void, Callable, Args...>::type;

	// STRUCT TEMPLATE is_invocable
	template <class Callable, class... Args>
	struct is_invocable : Invoke_traits<void, Callable, Args...>::Is_invocable
	{
		// determines whether Callable is callable with Args
	};

	template <class Callable, class... Args>
	inline constexpr bool is_invocable_v = Invoke_traits<void, Callable, Args...>::Is_invocable::value;

	// STRUCT TEMPLATE is_nothrow_invocable
	template <class Callable, class... Args>
	struct is_nothrow_invocable : Invoke_traits<void, Callable, Args...>::Is_nothrow_invocable
	{
		// determines whether Callable is nothrow-callable with Args
	};

	template <class Callable, class... Args>
	inline constexpr bool is_nothrow_invocable_v = Invoke_traits<void, Callable, Args...>::Is_nothrow_invocable::value;

	// STRUCT TEMPLATE is_invocable_r
	template <class Rx, class Callable, class... Args>
	struct is_invocable_r : Is_invocable_r_<Rx, Callable, Args...>
	{
		// determines whether Callable is callable with Args and return type Rx
	};

	template <class Rx, class Callable, class... Args>
	inline constexpr bool is_invocable_r_v = Is_invocable_r_<Rx, Callable, Args...>::value;

	// STRUCT TEMPLATE is_nothrow_invocable_r
	template <class Rx, class Callable, class... Args>
	struct is_nothrow_invocable_r : Invoke_traits<void, Callable, Args...>::template Is_nothrow_invocable_r<Rx>
	{
		// determines whether Callable is nothrow-callable with Args and return type Rx
	};

	template <class Rx, class Callable, class... Args>
	inline constexpr bool is_nothrow_invocable_r_v =
		Invoke_traits<void, Callable, Args...>::template Is_nothrow_invocable_r<Rx>::value;
}
