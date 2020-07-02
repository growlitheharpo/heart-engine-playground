#pragma once

#include "tiles/tile_spritesheet.h"

#include <heart/copy_move_semantics.h>
#include <heart/stl/unordered_map.h>
#include <heart/stl/utility.h>

#include <entt/entity/helper.hpp>

class Renderer;

using TileTag = entt::tag<"TileTag"_hs>;

class TileManager
{
private:
	hrt::vector<TileSpritesheet> m_spritesheets;
	hrt::unordered_map<uint32_t, hrt::pair<size_t, size_t>> m_spritemap;

#ifdef _DEBUG
	entt::entity m_selectedTile = entt::null;
#endif

	void PlaceTile(uint32_t tileKey, float x, float y);

public:
	TileManager() = default;
	DISABLE_COPY_SEMANTICS(TileManager);
	DISABLE_MOVE_SEMANTICS(TileManager);
	~TileManager() = default;

	void Initialize(const char* listPath);
	void LoadMap(void* data);
	void Dispose();

	void Render(Renderer& r);
};
