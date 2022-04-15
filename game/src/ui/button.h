/* Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
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
