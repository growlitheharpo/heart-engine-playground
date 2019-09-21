#include <SFML/Graphics.hpp>

#include <heart/stl/string.h>
#include <heart/stl/vector.h>

#include <heart/debug/assert.h>
#include <heart/debug/imgui.h>
#include <heart/debug/message_box.h>

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

#include "events/events.h"
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

	r.RegisterEvents();
	e.RegisterHandler(sf::Event::Closed, WindowClosedEvent);
	e.RegisterHandler(sf::Event::KeyPressed, EscapeKeyHitEvent);

	sf::Clock deltaClock;
	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);

	while (!s_shutdown)
	{
		e.Process();

#if IMGUI_ENABLED
		ImGui::Begin("Hello, world!");
		ImGui::Button("Look at this pretty button");
		ImGui::End();
#endif

		r.BeginFrame();
		r.Draw(shape);
		r.SubmitFrame();
	}

	e.Dispose();
	r.Dispose();

	return 0;
}
