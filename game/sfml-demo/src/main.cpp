#include "game/game.h"

#include "events/events.h"
#include "render/imgui_game.h"
#include "render/render.h"

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
	Renderer r;
	r.Initialize();

	EventManager& e = EventManager::Get();
	e.Initialize(&r);

	ImGui::Game::RegisterEvents();
	r.RegisterEvents();
	e.CreateHandler(sf::Event::Closed).connect<WindowClosedEvent>();
	e.CreateHandler(sf::Event::KeyPressed).connect<EscapeKeyHitEvent>();

	sf::Clock deltaClock;

	InitializeGame();

	constexpr uint64_t DESIRED_FRAME_TIME = 16667;

	while (!s_shutdown)
	{
		e.Process();

		auto elapsed = deltaClock.getElapsedTime().asMicroseconds();
		if (elapsed >= DESIRED_FRAME_TIME)
		{
			deltaClock.restart();
			RunGameTick(elapsed / float(1000000.0f));
		}

		r.BeginFrame();
		DrawGame(r);
		r.SubmitFrame();
	}

	ShutdownGame();

	e.Dispose();
	r.Dispose();

	return 0;
}
