#include "cmd_line.h"

#include "game/game.h"
#include "memory/memory.h"

#include "events/events.h"
#include "render/imgui_game.h"
#include "render/render.h"

#include "gen/gen.h"
#include "tween/tween_manager.h"

#include <heart/file.h>

static bool s_shutdown = false;

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

int WinMain()
{
	Memory::Init();

	auto commandLine = ParseCommandLine();

	// TODO: Move this to somewhere else (like the command line??)
	HeartSetRoot(commandLine["dataroot"].as<std::string>().c_str());
	ReflectSerializedData();

	Renderer& r = Renderer::Get();
	r.Initialize();

	EventManager& e = EventManager::Get();
	e.Initialize(&r);

	ImGui::Game::RegisterEvents();
	r.RegisterEvents();
	std::get<1>(e.CreateHandler(sf::Event::Closed)).connect<WindowClosedEvent>();
	std::get<1>(e.CreateHandler(sf::Event::KeyPressed)).connect<EscapeKeyHitEvent>();

	sf::Clock deltaClock;

	InitializeGame();

	constexpr uint64_t DESIRED_FRAME_TIME = 16667;

	uint64_t frameLimit = uint64_t(commandLine["framecount"].as<int>());
	uint64_t frameNumber = 0;

	while (!s_shutdown && frameNumber < frameLimit)
	{
		e.Process();

		auto elapsed = deltaClock.getElapsedTime();
		if (elapsed.asMicroseconds() >= DESIRED_FRAME_TIME)
		{
			deltaClock.restart();
			TweenManager::Tick(elapsed.asMilliseconds());
			RunGameTick(elapsed.asMicroseconds() / float(1000000.0f));

			++frameNumber;
		}

		r.BeginFrame();
		DrawGame(r);
		r.SubmitFrame();
	}

	ShutdownGame();
	sf::err() << "Succesfully ran " << frameNumber << " frames\n";

	e.Dispose();
	r.Dispose();

	return 0;
}
