/* Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
#include "cmd_line.h"

#include "game/game.h"
#include "memory/memory.h"

#include "events/events.h"
#include "render/imgui_game.h"
#include "render/render.h"

#include "gen/gen.h"
#include "tween/tween_manager.h"

#include <heart/file.h>

#include <heart/fibers/status.h>
#include <heart/fibers/system.h>
#include <heart/sync/event.h>

static bool s_shutdown = false;

static HeartEvent s_exitEvent;

static sf::Clock s_deltaClock = {};

constexpr uint64_t DESIRED_FRAME_TIME = 16667;

bool WindowClosedEvent(sf::Event e)
{
	s_shutdown = true;
	return true;
}

bool EscapeKeyHitEvent(sf::Event e)
{
	if (e.key.code != sf::Keyboard::Escape)
		return false;

	s_shutdown = true;
	return true;
}

HeartFiberStatus MainGameLoop(EventManager& e, Renderer& r, uint64_t& frameNumber, uint64_t frameLimit)
{
	e.Process();

	auto elapsed = s_deltaClock.getElapsedTime();
	if (elapsed.asMicroseconds() >= DESIRED_FRAME_TIME)
	{
		s_deltaClock.restart();
		TweenManager::Tick(elapsed.asMilliseconds());
		RunGameTick(elapsed.asMicroseconds() / float(1000000.0f));

		++frameNumber;
	}

	r.BeginFrame();
	DrawGame(r);
	r.SubmitFrame();

	if (!s_shutdown && frameNumber < frameLimit)
	{
		return HeartFiberStatus::Requeue;
	}
	else
	{
		s_exitEvent.Set();
		return HeartFiberStatus::Complete;
	}
}

int WinMain()
{
	Memory::Init();

	HeartFiberSystem::Settings fiberSettings;
	fiberSettings.threadCount = 1;
	Memory::BasePoolAllocator<byte_t, Memory::Pool::Fibers, Memory::Period::Long> fiberAllocator;

	HeartFiberSystem system(fiberAllocator);
	system.Initialize(fiberSettings);

	Renderer& r = Renderer::Get();
	EventManager& e = EventManager::Get();

	uint64_t frameLimit = 0;
	uint64_t frameNumber = 0;

	system.EnqueueFiber([&] {
		auto commandLine = ParseCommandLine();

		HeartSetRoot(commandLine["dataroot"].as<std::string>().c_str());

		ReflectSerializedData();

		frameLimit = uint64_t(commandLine["framecount"].as<int>());

		r.Initialize();

		e.Initialize(&r);

		ImGui::Game::RegisterEvents();
		r.RegisterEvents();
		std::get<1>(e.CreateHandler(sf::Event::Closed)).connect<WindowClosedEvent>();
		std::get<1>(e.CreateHandler(sf::Event::KeyPressed)).connect<EscapeKeyHitEvent>();

		InitializeGame();

		return HeartFiberStatus::Complete;
	});

	s_deltaClock.restart();

	system.EnqueueFiber([&] {
		return MainGameLoop(e, r, frameNumber, frameLimit);
	});

	// Put the "main thread" to sleep
	s_exitEvent.Wait();
	// We've woken up, time to shutdown the game

	system.EnqueueFiber([&] {
		ShutdownGame();

		sf::err() << "Succesfully ran " << frameNumber << " frames\n";

		e.Dispose();
		r.Dispose();

		return HeartFiberStatus::Complete;
	});

	system.Shutdown();

	return 0;
}
