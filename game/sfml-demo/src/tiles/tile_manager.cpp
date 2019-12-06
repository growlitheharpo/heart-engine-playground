#include "tile_manager.h"

#include "game/base_components.h"
#include "game/game.h"
#include "render/imgui_game.h"
#include "render/render.h"

#include <heart/deserialization_file.h>
#include <heart/scope_exit.h>

#include <SFML/Graphics.hpp>

#include <algorithm>

#if IMGUI_ENABLED
bool TileManagerImguiPanelActive = false;
#endif

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

#if IMGUI_ENABLED
		std::sort(&*entry.spritelist.begin(), &*entry.spritelist.end(),
			[](auto& lhs, auto& rhs) { return lhs.width * lhs.height < rhs.width * rhs.height; });
#endif
	}
}

void TileManager::Dispose()
{
	auto registry = GetRegistry();
	registry->view<TileTag>().each([registry](auto entity, auto tag) { registry->destroy(entity); });

	spritesheets_.clear();
}

void TileManager::Render(Renderer& r)
{
#if IMGUI_ENABLED
	if (spritesheets_.size() == 0)
		return;

	auto& sheet = spritesheets_[0];

	if (!ImGui::Game::IsActive() || !TileManagerImguiPanelActive)
		return;

	HEART_SCOPE_EXIT([]() { ImGui::End(); });
	if (ImGui::Begin("Tile Manager", &TileManagerImguiPanelActive))
	{
		const float regionWidth = ImGui::GetWindowContentRegionWidth();
		const float windowPadding = ImGui::GetStyle().WindowPadding.x;
		const float itemPadding = ImGui::GetStyle().FramePadding.x;

		float currentColumn = windowPadding;
		int prevSize = -1;

		for (auto& entry : sheet.spritelist)
		{
			float width = entry.width + itemPadding;
			int size = entry.width * entry.height;

			currentColumn += width;
			if (currentColumn + width >= regionWidth || size != prevSize)
				currentColumn = width + windowPadding;
			else
				ImGui::SameLine();

			prevSize = size;

			sf::IntRect rect(entry.x, entry.y, entry.width, entry.height);
			sf::Sprite sprite(sheet.texture, rect);
			ImGui::ImageButton(sprite);
		}
	}
#endif
}
