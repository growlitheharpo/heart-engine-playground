/* Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
#pragma once

#include <SFML/Graphics/Transform.hpp>

class Camera
{
private:
	sf::Vector2f m_position;

public:
	void SetPosition(sf::Vector2f p)
	{
		m_position.x = p.x;
		m_position.y = p.y;
	}

	sf::Vector2f GetPosition() const
	{
		return m_position;
	}

	sf::Transform GetTransform() const;

	sf::Vector2f ScreenToWorldPosition(sf::Vector2f screenPosition);
	sf::Vector2f WorldToScreenPosition(sf::Vector2f worldPosition);
};
