#pragma once

#include <SFML/System/Vector2.hpp>

#include <entt/entity/helper.hpp>
#include <entt/fwd.hpp>

namespace sf
{
	class Sprite;
	class Texture;
}

struct TransformableComponent
{
	sf::Vector2f position;
	sf::Vector2f rotation;
};

struct DrawableComponent
{
	sf::Sprite* sprite = nullptr;
	sf::Texture* texture = nullptr;
	float z = 0.0f;

	static void OnDestroy(entt::registry& r, entt::entity e);
};

struct InputStatusComponent
{
	uint8_t flags = 0;
};

using PlayerTag = entt::tag<"PlayerTag"_hs>;
