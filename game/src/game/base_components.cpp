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
