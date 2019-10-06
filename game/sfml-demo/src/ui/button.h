#pragma once

#include "events/events.h"
#include "ui/widget.h"

#include <heart/deserialization_fwd.h>

namespace UI
{
	class Button : public Widget
	{
	public:
		SERIALIZE_STRUCT()
		struct ButtonData
		{
			float offsetX, offsetY;
			float sizeX, sizeY;
			int offsetSizeMode;
			SerializedString<32> label;
			SerializedString<48> action;
		};

	private:
		EventManager::EventFilterFunc click_event_handle_;
		ButtonData data_;

		bool OnClick(sf::Event e);

	public:
		~Button();

		// Inherited via Widget
		virtual void Initialize() override;

		virtual void Destroy() override;

		virtual void Update() override;

		virtual void Draw() const override;
	};
}
