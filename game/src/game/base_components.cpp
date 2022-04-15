/* Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
#include "base_components.h"

#include <game/game.h>

#include <SFML/Graphics.hpp>

void DrawableComponent::OnDestroy(entt::registry& r, entt::entity e)
{
	auto& d = r.get<DrawableComponent>(e);
	delete d.sprite;
	d.sprite = nullptr;

	delete d.texture;
	d.texture = nullptr;
}
