/* Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
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
