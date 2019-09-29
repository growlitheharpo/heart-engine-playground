#pragma once

#include "ui/widget.h"
#include "events/events.h"

namespace UI
{
	class Button : public Widget
	{
	private:
		EventManager::EventFilterFunc event_handle_;

	public:
		~Button();

		// Inherited via Widget
		virtual void Initialize() const override;

		virtual void Update() const override;

		virtual void Draw() const override;
	};
}
