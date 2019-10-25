#pragma once

#include <heart/deserialization_fwd.h>
#include <heart/stl/vector.h>

class Renderer;

namespace UI
{
	class Widget;

	SERIALIZE_STRUCT()
	struct GlobalButtonFunctionality
	{
		static GlobalButtonFunctionality Instance()
		{
			return GlobalButtonFunctionality();
		}

		SERIALIZE_MEMBER_METHOD() static bool CloseGame();
		SERIALIZE_MEMBER_METHOD() static bool TogglePause();
		SERIALIZE_MEMBER_METHOD() static bool ResetPlayer();
	};

	class UIManager
	{
	private:
		hrt::vector<UI::Widget*> widgets_;

	public:
		void Initialize();
		void Cleanup();

		void LoadPanel(const char* panelName);

		void Update();
		void Render(Renderer& r);
	};
}
