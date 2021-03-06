#include "camera.h"

#include <render/render.h>

sf::Transform Camera::GetTransform() const
{
	return sf::Transform(
		1.0f, 0.0f, -m_position.x,
		0.0f, -1.0f, m_position.y + Renderer::Get().GetScreenSize().y,
		0.0f, 0.0f, 1.0f);
}

sf::Vector2f Camera::ScreenToWorldPosition(sf::Vector2f screenPosition)
{
	return GetTransform().getInverse().transformPoint(screenPosition);
}

sf::Vector2f Camera::WorldToScreenPosition(sf::Vector2f worldPosition)
{
	return GetTransform().transformPoint(worldPosition);
}
