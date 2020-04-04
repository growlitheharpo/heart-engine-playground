#pragma once

#include <SFML/Graphics/Transform.hpp>

class Camera
{
private:
	sf::Vector2f position_;

public:
	void SetPosition(sf::Vector2f p)
	{
		position_.x = p.x;
		position_.y = p.y;
	}

	sf::Vector2f GetPosition() const
	{
		return position_;
	}

	sf::Transform GetTransform() const;

	sf::Vector2f ScreenToWorldPosition(sf::Vector2f screenPosition);
	sf::Vector2f WorldToScreenPosition(sf::Vector2f worldPosition);
};
