#include "button.h"

#include "render/render.h"
#include "ui/ui_manager.h"

#include <entt/core/hashed_string.hpp>
#include <entt/meta/factory.hpp>
#include <entt/meta/meta.hpp>

static bool CheckBounds(sf::IntRect buttonRect)
{
	auto mousePos = RenderUtils::GetMousePosition();
	return buttonRect.contains(mousePos);
}

bool UI::Button::OnClick(sf::Event e)
{
	if (e.mouseButton.button != sf::Mouse::Left)
		return false;

	auto rect = GetRect();
	bool hit = CheckBounds(rect);

	if (!hit)
		return false;

	// We only need to resolve the type once, so make it static
	static auto metaType = entt::resolve<UI::GlobalButtonFunctionality>();
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

sf::IntRect UI::Button::GetRect() const
{
	return sf::IntRect(int(data_.offsetX), int(data_.offsetY), int(data_.sizeX), int(data_.sizeY));
	return sf::IntRect();
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

void UI::Button::Draw(Renderer& r) const
{
	sf::RectangleShape rs;

	sf::Vector2f origin(data_.offsetX, data_.offsetY);
	sf::Vector2f size(data_.sizeX, data_.sizeY);

	rs.setPosition(origin);
	rs.setSize(size);
	rs.setFillColor(sf::Color::White);

	r.Draw(rs);
}
