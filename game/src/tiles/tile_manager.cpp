#include "tile_manager.h"

#include "events/events.h"
#include "game/base_components.h"
#include "game/game.h"
#include "render/imgui_game.h"
#include "render/render.h"

#include <heart/scope_exit.h>

#include <heart/deserialization/deserialization_file.h>
#include <heart/stl/string.h>

#include <SFML/Graphics.hpp>
#include <icons/IconsKenney.h>
#include <smhasher/src/MurmurHash3.h>

#include <algorithm>

#if IMGUI_ENABLED
bool TileManagerImguiPanelActive = false;
#endif

constexpr uint32_t TileSeed = 0x288af4b2ULL;

void TileManager::PlaceTile(uint32_t tileKey, float x, float y)
{
	auto iter = m_spritemap.find(tileKey);
	if (iter == m_spritemap.end())
		return;

	size_t sheetIndex = iter->second.first;
	size_t tileIndex = iter->second.second;

	auto& sheet = m_spritesheets[sheetIndex];
	auto& tile = sheet.spritelist[tileIndex];

	auto newTile = create_multi_component<TransformComponent, DrawableComponent, TileTag>();

	auto& pos = std::get<1>(newTile);
	pos.position.x = x;
	pos.position.y = y;

	auto& draw = std::get<2>(newTile);
	draw.texture = nullptr; // the tile manager cleans up its textures
	draw.sprite = new sf::Sprite(sheet.texture, sf::IntRect(tile.x, tile.y, tile.width, tile.height));
}

void TileManager::Initialize(const char* listPath)
{
	TileSpritesheetList tilesets;
	HeartDeserializeObjectFromFile<TileSpritesheetList, Memory::UIShortAllocator<uint8_t>>(tilesets, "json/tileset_list.json");

	for (auto& filename : tilesets.filelist)
	{
		auto& entry = m_spritesheets.emplace_back();
		if (!HeartDeserializeObjectFromFile<TileSpritesheet, Memory::UIShortAllocator<uint8_t>>(entry, filename.c_str()))
		{
			m_spritesheets.pop_back();
			continue;
		}

		char texturePath[64];
		strcpy_s(texturePath, filename.c_str());
		auto extension = strrchr(texturePath, '.');
		if (extension == nullptr)
		{
			m_spritesheets.pop_back();
			continue;
		}

		strcpy_s(extension, texturePath + sizeof(texturePath) - extension, ".png");
		HEART_CHECK(RenderUtils::LoadTextureFromFile(entry.texture, texturePath));

		std::sort(&*entry.spritelist.begin(), &*entry.spritelist.end(), [](auto& lhs, auto& rhs) { return lhs.width * lhs.height < rhs.width * rhs.height; });
	}

	for (size_t i = 0; i < m_spritesheets.size(); ++i)
	{
		auto& sheet = m_spritesheets[i];

		for (size_t j = 0; j < sheet.spritelist.size(); ++j)
		{
			auto& entry = sheet.spritelist[j];

			hrt::string hashedName = tilesets.filelist[i].c_str();
			hashedName.append(entry.name.c_str());

			MurmurHash3_x86_32(hashedName.c_str(), int(hashedName.size()), TileSeed, &entry.key);

			HEART_ASSERT(m_spritemap.find(entry.key) == m_spritemap.end(), "HASH COLLISION IN TILE MANAGER!");
			m_spritemap[entry.key] = hrt::make_pair(i, j);
		}
	}
}

void TileManager::LoadMap(void* data)
{
	HEART_ASSERT(data != nullptr);

	uint32_t* stream = (uint32_t*)data;
	uint32_t width = *stream++;
	uint32_t height = *stream++;
	for (uint32_t y = 0; y < height; ++y)
	{
		for (uint32_t x = 0; x < width; ++x)
		{
			uint32_t tileKey = *stream++;
			PlaceTile(tileKey, x * 64.0f, y * 32.0f);
		}
	}
}

void TileManager::Dispose()
{
	auto& registry = GetRegistry();
	registry.view<TileTag>().each([&registry](auto entity) { registry.destroy(entity); });

	m_spritesheets.clear();
}

void TileManager::Render(Renderer& r)
{
#if IMGUI_ENABLED
	if (m_spritesheets.size() == 0)
		return;

	if (!ImGui::Game::IsActive() || !TileManagerImguiPanelActive)
		return;

	constexpr char TileDropId[] = "TILEID";

	{
		// drop target
		ImGui::SetNextWindowPos(sf::Vector2f(0.0f, 0.0f));
		ImGui::SetNextWindowSize(r.GetScreenSize());
		if (ImGui::Begin("##DropTarget", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::Dummy(r.GetScreenSize());
			if (ImGui::BeginDragDropTarget())
			{
				if (auto payload = ImGui::AcceptDragDropPayload(TileDropId))
				{
					uint32_t tileKey = *(uint32_t*)payload->Data;

					sf::Vector2f screenPos = ImGui::GetMousePos();
					sf::Vector2f worldPos = r.GetCameraRef().ScreenToWorldPosition(screenPos);

					// TODO: move into grid space
					worldPos.y -= 64.0f;

					PlaceTile(tileKey, worldPos.x, worldPos.y);
				}

				ImGui::EndDragDropTarget();
			}
		}

		ImGui::End();
	}

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
			for (auto& sheet : m_spritesheets)
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
					ImGui::ImageButton(sprite);

					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
					{
						ImGui::SetDragDropPayload(TileDropId, &entry.key, sizeof(entry.key));
						ImGui::EndDragDropSource();
					}

					ImGui::PopID();
				}
			}
		}
	}

	/*
	const float InitialPos = 220.0f;

	static float xAdjust = 0.0f, yAdjust = 0.0f;
	static bool drawGrid = false;

	if (drawGrid)
	{
		// large grid is 64x32
		auto size = r.GetScreenSize();
		static float LineWidth = 2.0f;

		static int HorizontalStep = 64;
		static float HorizontalAngle = atan(32.0f / 64.0f) * 180.0f / 3.141592654f;

		int horizontalCount = int(size.y) / HorizontalStep;
		for (int horizontal = -horizontalCount; horizontal < horizontalCount * 2; ++horizontal)
		{
			sf::RectangleShape line(sf::Vector2f(size.x * 3.0f, LineWidth));
			line.setPosition(sf::Vector2f(0.0f, float(horizontal * HorizontalStep)));
			line.setFillColor(sf::Color::White);
			line.rotate(HorizontalAngle);
			r.Draw(line);
		}

		static int VerticalStep = 128;
		float VerticalAngle = 90.0f - HorizontalAngle;

		int verticalCount = int(size.x) / VerticalStep;
		for (int vertical = -verticalCount; vertical < verticalCount * 2; ++vertical)
		{
			sf::RectangleShape line(sf::Vector2f(LineWidth, size.y * 3.0f));
			line.setPosition(sf::Vector2f(float(vertical * VerticalStep), 0.0f));
			line.setFillColor(sf::Color::White);
			line.rotate(VerticalAngle);
			r.Draw(line);
		}
	}

	{
		HEART_SCOPE_EXIT([]() { ImGui::End(); });
		if (ImGui::Begin("Tile Manager", &TileManagerImguiPanelActive))
		{
			ImGui::Checkbox("Draw Grid", &drawGrid);

			const float regionWidth = ImGui::GetWindowContentRegionWidth();
			const float windowPadding = ImGui::GetStyle().WindowPadding.x;
			const float itemPadding = ImGui::GetStyle().FramePadding.x;

			float currentColumn = windowPadding;
			int prevSize = -1;

			int id = 0;
			for (auto& sheet : m_spritesheets)
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
						auto newTile = GetRegistry()->create<TileTag, TransformComponent, DrawableComponent>();
						m_selectedTile = std::get<0>(newTile);

						auto& pos = std::get<2>(newTile);
						pos.position = {InitialPos, InitialPos};

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

	if (m_selectedTile != entt::null)
	{
		HEART_SCOPE_EXIT([]() { ImGui::End(); });
		if (ImGui::Begin("Selected Sprite"))
		{
			if (ImGui::Button("Move Back"))
				GetRegistry()->get<DrawableComponent>(m_selectedTile).z -= 1.0f;
			if (ImGui::Button("Move Up"))
				GetRegistry()->get<DrawableComponent>(m_selectedTile).z += 1.0f;

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

			auto& transform = GetRegistry()->get<TransformComponent>(m_selectedTile);
			transform.position.x = InitialPos + xAdjust;
			transform.position.y = InitialPos + yAdjust;
		}
	}
	/**/

#endif
}
