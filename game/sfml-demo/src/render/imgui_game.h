#pragma once

namespace sf
{
	class RenderWindow;
	class Event;
}

namespace ImGui
{
	namespace Game
	{
		void Init(sf::RenderWindow* i);
		void ProcessEvent(const sf::Event& e);
		void Tick(sf::RenderWindow* i);
		void BeginRender();
		void SubmitRender(sf::RenderWindow* i);
		void Shutdown();

		bool IsActive();
		void RegisterEvents();
	}
}
