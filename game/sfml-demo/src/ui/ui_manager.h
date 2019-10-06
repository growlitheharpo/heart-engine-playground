#pragma once

#include <heart/deserialization_fwd.h>

namespace UI
{
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
	public:
	};
}
