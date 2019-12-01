#include "tile_manager.h"

#include "game/base_components.h"
#include "game/game.h"
#include "render/render.h"

#include <heart/deserialization_file.h>

#include <SFML/Graphics.hpp>

void TileManager::Initialize(const char* listPath)
{
	TileSpritesheetList tilesets;
	HeartDeserializeObjectFromFile(tilesets, "json/tileset_list.json");

	for (auto& filename : tilesets.filelist)
	{
		auto& entry = spritesheets_.emplace_back();
		if (!HeartDeserializeObjectFromFile(entry, filename.c_str()))
		{
			spritesheets_.pop_back();
			continue;
		}

		char texturePath[64];
		strcpy_s(texturePath, filename.c_str());
		auto extension = strrchr(texturePath, '.');
		if (extension == nullptr)
		{
			spritesheets_.pop_back();
			continue;
		}

		strcpy_s(extension, texturePath + sizeof(texturePath) - extension, ".png");
		HEART_CHECK(RenderUtils::LoadTextureFromFile(entry.texture, texturePath));
	}
}

void TileManager::Dispose()
{
	auto registry = GetRegistry();
	registry->view<TileTag>().each([registry](auto entity, auto tag) { registry->destroy(entity); });

	spritesheets_.clear();
}
