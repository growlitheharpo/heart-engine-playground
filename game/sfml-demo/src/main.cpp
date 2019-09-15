#include <SFML/Graphics.hpp>

#include <heart/stl/vector.h>
#include <heart/stl/string.h>

#include <heart/debug/assert.h>
#include <heart/debug/message_box.h>
#include <heart/debug/imgui.h>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>


int WinMain()
{
	sf::RenderWindow window(sf::VideoMode(800, 600), "SFML works!", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);

#if IMGUI_ENABLED
	ImGui::SFML::Init(window);
#endif

	sf::Clock deltaClock;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
#if IMGUI_ENABLED
			ImGui::SFML::ProcessEvent(event);
#endif

			if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
			{
				window.close();
			}
			else if (event.type == sf::Event::Resized)
			{
				sf::FloatRect visibleArea(0, 0, float(event.size.width), float(event.size.height));
				window.setView(sf::View(visibleArea));
			}
		}

#if IMGUI_ENABLED
		ImGui::SFML::Update(window, deltaClock.restart());
#endif

#if IMGUI_ENABLED
		ImGui::Begin("Hello, world!");
		ImGui::Button("Look at this pretty button");
		ImGui::End();
#endif

		window.clear();
		window.draw(shape);

#if IMGUI_ENABLED
		ImGui::SFML::Render(window);
#endif

		window.display();
	}

#if IMGUI_ENABLED
	ImGui::SFML::Shutdown();
#endif

	return 0;
}
