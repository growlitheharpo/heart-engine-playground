#pragma once

#include "events/events.h"
#include "ui/widget.h"

#include <heart/codegen/codegen.h>

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
		EventManager::EventFuncHandle m_clickEventHandle;
		ButtonData m_data;

		bool OnClick(sf::Event e);

		sf::IntRect GetRect() const;

	public:
		Button(ButtonData d) :
			m_data(d)
		{
		}

		~Button()
		{
			Destroy();
		}

		// Inherited via Widget
		virtual void Initialize() override;

		virtual void Destroy() override;

		virtual void Update() override;

		virtual void Draw(Renderer& r) const override;
	};
}
