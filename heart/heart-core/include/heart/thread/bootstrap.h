/* Copyright (C) 2023 James Keats
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

#include <heart/types.h>

#include <heart/function/embedded_function.h>
#include <heart/thread/thread.h>

#include <heart/sleep.h>

#include <atomic>
#include <tuple>

namespace heart_priv
{
	template <typename TargetT, typename... ArgsT>
	struct HeartThreadBootstrapPackage
	{
		typedef void (TargetT::*FunctionPtr)(ArgsT...);

		enum State
		{
			Setup,
			Ready,
			Done,
		};

		std::atomic<State> state = State::Setup;

		TargetT* target = nullptr;

		FunctionPtr funcPtr = nullptr;

		std::tuple<ArgsT...> arguments;
	};

	template <typename TargetT, typename... ArgsT>
	void* HeartThreadMemberBootstrapEntrypoint(void* vparam)
	{
		using BootstrapPackage = typename heart_priv::HeartThreadBootstrapPackage<TargetT, ArgsT...>;
		BootstrapPackage* parameter = static_cast<BootstrapPackage*>(vparam);
		while (parameter->state != BootstrapPackage::Ready)
		{
			HeartYield();
		}

		std::tuple<ArgsT...> arguments = std::move(parameter->arguments);
		auto func = parameter->funcPtr;
		auto target = parameter->target;

		parameter->state = BootstrapPackage::Done;
		parameter = nullptr;

		auto stim = [target, func](auto&&... args) {
			std::invoke(func, *target, std::forward<ArgsT>(args)...);
		};

		std::apply(stim, std::move(arguments));
		return nullptr;
	}
}

template <typename TargetT, typename... ArgsT>
HeartThread HeartThreadMemberBootstrap(HeartThread::Priority priority, uint32 stackSize, TargetT* object, void (TargetT::*funcPtr)(ArgsT...), ArgsT... args)
{
	using BootstrapPackage = typename heart_priv::HeartThreadBootstrapPackage<TargetT, ArgsT...>;
	BootstrapPackage package;
	package.state = BootstrapPackage::Setup;
	package.target = object;
	package.funcPtr = funcPtr;
	package.arguments = std::tuple<ArgsT...>(hrt::forward<ArgsT>(args)...);

	HeartThread result = HeartThread(&heart_priv::HeartThreadMemberBootstrapEntrypoint<TargetT, ArgsT...>, &package, priority, stackSize);
	package.state = BootstrapPackage::Ready;
	while (package.state != BootstrapPackage::Done)
		HeartYield();

	return result;
}

template <typename TargetT, typename... ArgsT>
HeartThread HeartThreadMemberBootstrap(TargetT* object, void (TargetT::*funcPtr)(ArgsT...), ArgsT... args)
{
	return HeartThreadMemberBootstrap(HeartThread::Priority::Normal, 0, object, funcPtr, hrt::forward<ArgsT>(args)...);
}

template <typename TargetT, typename... ArgsT>
HeartThread HeartThreadMemberBootstrap(HeartThread::Priority priority, TargetT* object, void (TargetT::*funcPtr)(ArgsT...), ArgsT... args)
{
	return HeartThreadMemberBootstrap(priority, 0, object, funcPtr, hrt::forward<ArgsT>(args)...);
}
