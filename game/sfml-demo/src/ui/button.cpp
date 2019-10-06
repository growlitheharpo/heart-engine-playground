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
	auto click = EventManager::Get().CreateHandler(sf::Event::MouseButtonPressed);
	click_event_handle_ = hrt::get<0>(click);
	hrt::get<1>(click).connect<&UI::Button::OnClick>(*this);
}

void UI::Button::Destroy()
{
	EventManager::Get().RemoveHandler(click_event_handle_);
}

void UI::Button::Update()
{
}

void UI::Button::Draw() const
{
}
