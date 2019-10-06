#include "button.h"

#include "ui/ui_manager.h"

#include <entt/core/hashed_string.hpp>
#include <entt/meta/factory.hpp>
#include <entt/meta/meta.hpp>

bool UI::Button::OnClick(sf::Event e)
{
	if (e.mouseButton.button != sf::Mouse::Left)
		return false;

	// TODO: Bounds check! (duh)
	bool hit = true;

	if (!hit)
		return false;

	auto metaType = entt::resolve<UI::GlobalButtonFunctionality>();
	if (!metaType)
		return false;

	auto func = metaType.func(entt::hashed_string::to_value(data_.action.Get()));
	if (!func)
		return false;

	auto result = func.invoke(GlobalButtonFunctionality::Instance());
	if (result)
		return true;

	return false;
}

void UI::Button::Initialize()
{
	click_event_handle_ = EventManager::Get().CreateHandler(sf::Event::MouseButtonPressed);
	click_event_handle_.connect<&UI::Button::OnClick>(*this);
}

void UI::Button::Destroy()
{
	// TODO: Actually remove the handler from the map!
	click_event_handle_.reset();
}

void UI::Button::Update()
{
}

void UI::Button::Draw() const
{
}
