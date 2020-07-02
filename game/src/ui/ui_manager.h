#pragma once

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
		hrt::vector<UI::Widget*> m_widgets;
		hrt::unordered_map<hrt::string, sf::Font*> m_loadedFonts;

	public:
		void Initialize();
		void Cleanup();

		void LoadPanel(const char* panelName);

		sf::Font* FindOrLoadFont(const char* fontName);

		void Update();
		void Render(Renderer& r);
	};
}
