#include "camera.h"

#include <render/render.h>

sf::Transform Camera::GetTransform() const
{
	sf::Transform tf(1.0f, 0.0f, -position_.x,
		0.0f, -1.0f, position_.y + Renderer::Get().GetScreenSize().y,
		0.0f, 0.0f, 1.0f);
	return tf;
}

sf::Vector2f Camera::ScreenToWorldPosition(sf::Vector2f screenPosition)
{
	return GetTransform().getInverse().transformPoint(screenPosition);
}

sf::Vector2f Camera::WorldToScreenPosition(sf::Vector2f worldPosition)
{
	return GetTransform().transformPoint(worldPosition);
}
