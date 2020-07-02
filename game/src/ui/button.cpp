#include "button.h"

#include "render/render.h"
#include "ui/ui_manager.h"

#include <entt/core/hashed_string.hpp>
#include <entt/meta/factory.hpp>
#include <entt/meta/meta.hpp>
#include <entt/meta/resolve.hpp>

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

	auto func = metaType.func(entt::hashed_string::value(m_data.action.Get()));
	if (!func)
		return false;

	auto result = func.invoke(GlobalButtonFunctionality::Instance());
	if (result)
		return true;

	return false;
}

sf::IntRect UI::Button::GetRect() const
{
	return sf::IntRect(int(m_data.offsetX), int(m_data.offsetY), int(m_data.sizeX), int(m_data.sizeY));
	return sf::IntRect();
}

void UI::Button::Initialize()
{
	auto click = EventManager::Get().CreateHandler(sf::Event::MouseButtonPressed);
	m_clickEventHandle = hrt::get<0>(click);
	hrt::get<1>(click).connect<&UI::Button::OnClick>(*this);
}

void UI::Button::Destroy()
{
	EventManager::Get().RemoveHandler(m_clickEventHandle);
}

void UI::Button::Update()
{
}

void UI::Button::Draw(Renderer& r) const
{
	sf::RectangleShape rs;

	sf::Vector2f origin(m_data.offsetX, m_data.offsetY);
	sf::Vector2f size(m_data.sizeX, m_data.sizeY);

	rs.setPosition(origin);
	rs.setSize(size);
	rs.setFillColor(sf::Color::White);

	r.Draw(rs);
}
