#include "imgui_game.h"
#include "events/events.h"

#include "icons/IconsKenney.h"

#include <heart/debug/imgui.h>
#include <heart/util/file_load.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#if IMGUI_ENABLED
static bool s_imgui_active = false;
static bool s_was_ever_initialized = false;
static bool s_awaiting_render = false;
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

			auto fontData = HeartUtilLoadExistingFile("fonts/" FONT_ICON_FILE_NAME_KI);
			if (!fontData.empty())
			{
				auto& io = ImGui::GetIO();

				ImFontConfig config;
				config.MergeMode = true;
				config.FontDataOwnedByAtlas = false;
				config.GlyphMinAdvanceX = 13.0f;

				static const ImWchar icon_ranges[] = {ICON_MIN_KI, ICON_MAX_KI, 0};

				io.Fonts->AddFontFromMemoryTTF(fontData.data(), int(fontData.size()), 13.0f, &config, icon_ranges);
				io.Fonts->Build();
				ImGui::SFML::UpdateFontTexture();
			}
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

			if (s_awaiting_render)
				ImGui::EndFrame();

			static sf::Clock imGuiClock;
			ImGui::SFML::Update(*i, imGuiClock.restart());
			s_awaiting_render = true;
#endif
		}

		void SubmitRender(sf::RenderWindow* i)
		{
#if IMGUI_ENABLED
			if (!IsActive() || !s_was_ever_initialized || i == nullptr)
				return;

			ImGui::SFML::Render(*i);
			s_awaiting_render = false;
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
