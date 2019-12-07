#include "tile_manager.h"

#include "events/events.h"
#include "game/base_components.h"
#include "game/game.h"
#include "render/imgui_game.h"
#include "render/render.h"

#include <heart/deserialization_file.h>
#include <heart/scope_exit.h>

#include <SFML/Graphics.hpp>
#include <icons/IconsKenney.h>

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

	if (!ImGui::Game::IsActive() || !TileManagerImguiPanelActive)
		return;

	static float xAdjust = 0.0f, yAdjust = 0.0f;

	{
		HEART_SCOPE_EXIT([]() { ImGui::End(); });
		if (ImGui::Begin("Tile Manager", &TileManagerImguiPanelActive))
		{
			const float regionWidth = ImGui::GetWindowContentRegionWidth();
			const float windowPadding = ImGui::GetStyle().WindowPadding.x;
			const float itemPadding = ImGui::GetStyle().FramePadding.x;

			float currentColumn = windowPadding;
			int prevSize = -1;

			int id = 0;
			for (auto& sheet : spritesheets_)
			{
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
					ImGui::PushID(id++);
					if (ImGui::ImageButton(sprite))
					{
						auto newTile = GetRegistry()->create<TileTag, TransformableComponent, DrawableComponent>();
						selected_tile_ = std::get<0>(newTile);

						auto& pos = std::get<2>(newTile);
						pos.position = {150.0f, 150.0f};

						auto& drawable = std::get<3>(newTile);
						drawable.texture = nullptr; // the tile manager cleans up its textures
						drawable.sprite = new sf::Sprite(sheet.texture, rect);

						xAdjust = 0.0f;
						yAdjust = 0.0f;
					}
					ImGui::PopID();
				}
			}
		}
	}

	if (selected_tile_ != entt::null)
	{
		HEART_SCOPE_EXIT([]() { ImGui::End(); });
		if (ImGui::Begin("Selected Sprite"))
		{
			if (ImGui::Button("Move Back"))
				GetRegistry()->get<DrawableComponent>(selected_tile_).z -= 1.0f;
			if (ImGui::Button("Move Up"))
				GetRegistry()->get<DrawableComponent>(selected_tile_).z += 1.0f;

			// big tile footprint for iso buildings: 64x32
			// little tiles have an x offset of 16 from their base
			static int xStep = 32;
			ImGui::SliderInt("Step Size", &xStep, 1, 32);
			int yStep = xStep / 2;

			const static sf::Vector2f buttonSize(24.0f, 24.0f);
			if (ImGui::Button(ICON_KI_ARROW_TOP_LEFT, buttonSize))
			{
				xAdjust -= xStep;
				yAdjust += yStep;
			}
			ImGui::SameLine();
			if (ImGui::Button(ICON_KI_ARROW_TOP, buttonSize))
			{
				yAdjust += yStep;
			}
			ImGui::SameLine();
			if (ImGui::Button(ICON_KI_ARROW_TOP_RIGHT, buttonSize))
			{
				xAdjust += xStep;
				yAdjust += yStep;
			}

			if (ImGui::Button(ICON_KI_ARROW_LEFT, buttonSize))
			{
				xAdjust -= xStep;
			}
			ImGui::SameLine();
			ImGui::Button(" ", buttonSize);
			ImGui::SameLine();
			if (ImGui::Button(ICON_KI_ARROW_RIGHT, buttonSize))
			{
				xAdjust += xStep;
			}

			if (ImGui::Button(ICON_KI_ARROW_BOTTOM_LEFT, buttonSize))
			{
				xAdjust -= xStep;
				yAdjust -= yStep;
			}
			ImGui::SameLine();
			if (ImGui::Button(ICON_KI_ARROW_BOTTOM, buttonSize))
			{
				yAdjust -= yStep;
			}
			ImGui::SameLine();
			if (ImGui::Button(ICON_KI_ARROW_BOTTOM_RIGHT, buttonSize))
			{
				xAdjust += xStep;
				yAdjust -= yStep;
			}

			ImGui::Text("X: %.0f Y: %.0f", xAdjust, yAdjust);

			auto& transform = GetRegistry()->get<TransformableComponent>(selected_tile_);
			transform.position.x = 150.0f + xAdjust;
			transform.position.y = 150.0f + yAdjust;
		}
	}
#endif
}
