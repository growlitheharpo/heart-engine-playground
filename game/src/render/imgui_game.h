/* Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
#pragma once

#include <heart/debug/imgui.h>

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
		void SubmitRender(sf::RenderWindow* i);
		void Shutdown();

		bool IsActive();
		void RegisterEvents();
	}
}
