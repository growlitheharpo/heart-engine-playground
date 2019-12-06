#pragma once

#include "tiles/tile_spritesheet.h"

#include <heart/copy_move_semantics.h>

#include <entt/entity/helper.hpp>

class Renderer;

using TileTag = entt::tag<"TileTag"_hs>;

class TileManager
{
private:
	hrt::vector<TileSpritesheet> spritesheets_;

public:
	TileManager() = default;
	DISABLE_COPY_SEMANTICS(TileManager);
	DISABLE_MOVE_SEMANTICS(TileManager);
	~TileManager() = default;

	void Initialize(const char* listPath);
	void Dispose();

	void Render(Renderer& r);
};
