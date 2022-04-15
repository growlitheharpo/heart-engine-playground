/* Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
#pragma once

#include "tiles/tile_spritesheet.h"

#include "memory/memory.h"

#include <heart/copy_move_semantics.h>
#include <heart/stl/unordered_map.h>
#include <heart/stl/utility.h>

#include <entt/entity/helper.hpp>

class Renderer;

using TileTag = entt::tag<entt::hashed_string {"TileTag"}>;

class TileManager
{
private:
	hrt::vector<TileSpritesheet, Memory::UILongAllocator<TileSpritesheet>> m_spritesheets;
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
