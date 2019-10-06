#include "imgui_game.h"
#include "events/events.h"

#include <heart/debug/imgui.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#if IMGUI_ENABLED
static bool s_imgui_active = false;
static bool s_was_ever_initialized = false;
#endif

namespace ImGui
{
	namespace Game
	{
		void Init(sf::RenderWindow* i)
		{
#if IMGUI_ENABLED
			if (i == nullptr || s_was_ever_initialized)
				return;

			s_was_ever_initialized = true;
			ImGui::SFML::Init(*i);
#endif
		}

		void ProcessEvent(const sf::Event& e)
		{
#if IMGUI_ENABLED
			if (!IsActive() || !s_was_ever_initialized)
				return;

			ImGui::SFML::ProcessEvent(e);
#endif
		}

		void Tick(sf::RenderWindow* i)
		{
#if IMGUI_ENABLED
			if (!IsActive() || !s_was_ever_initialized || i == nullptr)
				return;

			static sf::Clock imGuiClock;
			ImGui::SFML::Update(*i, imGuiClock.restart());
#endif
		}

		void BeginRender()
		{
#if IMGUI_ENABLED
			if (!IsActive() || !s_was_ever_initialized)
				return;

			ImGui::EndFrame();
			ImGui::NewFrame();
#endif
		}

		void SubmitRender(sf::RenderWindow* i)
		{
#if IMGUI_ENABLED
			if (!IsActive() || !s_was_ever_initialized || i == nullptr)
				return;

			ImGui::SFML::Render(*i);
#endif
		}

		void Shutdown()
		{
#if IMGUI_ENABLED
			if (!s_was_ever_initialized)
				return;

			ImGui::SFML::Shutdown();
#endif
		}

		bool IsActive()
		{
#if IMGUI_ENABLED
			return s_imgui_active;
#else
			return false;
#endif
		}

#if IMGUI_ENABLED
		static bool sToggleImGuiEvent(sf::Event e)
		{
			if (e.key.code != sf::Keyboard::F3)
				return false;

			s_imgui_active = !s_imgui_active;
			return true;
		}
#endif

		void RegisterEvents()
		{
#if IMGUI_ENABLED
			hrt::get<1>(EventManager::Get().CreateHandler(sf::Event::KeyPressed)).connect<sToggleImGuiEvent>();
#endif
		}
	}
}
