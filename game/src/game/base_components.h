/* Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
#pragma once

#include <SFML/System/Vector2.hpp>

#include <entt/entity/helper.hpp>
#include <entt/fwd.hpp>

namespace sf
{
	class Sprite;
	class Texture;
}

struct TransformComponent
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

using PlayerTag = entt::tag<entt::hashed_string {"PlayerTag"}>;
