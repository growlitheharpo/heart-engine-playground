#pragma once

#include "memory/memory.h"

#include <heart/codegen/codegen.h>

#include <heart/stl/string.h>
#include <heart/stl/unordered_map.h>
#include <heart/stl/vector.h>

class Renderer;

namespace sf
{
	class Font;
}

namespace UI
{
	class Widget;

	SERIALIZE_STRUCT()
	struct GlobalButtonFunctionality
	{
		static GlobalButtonFunctionality& Instance()
		{
			static GlobalButtonFunctionality gbf;
			return gbf;
		}

		SERIALIZE_MEMBER_METHOD()
		static bool CloseGame();

		SERIALIZE_MEMBER_METHOD()
		static bool TogglePause();

		SERIALIZE_MEMBER_METHOD()
		static bool ResetPlayer();
	};

	class UIManager
	{
	private:
		hrt::vector_a<UI::Widget*, Memory::UILongAllocator> m_widgets;
		hrt::unordered_map_a<hrt::string, sf::Font*, Memory::UILongAllocator> m_loadedFonts;

	public:
		void Initialize();
		void Cleanup();

		void LoadPanel(const char* panelName);

		sf::Font* FindOrLoadFont(const char* fontName);

		void Update();
		void Render(Renderer& r);
	};
}
