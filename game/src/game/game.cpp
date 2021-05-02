#include "game.h"
#include "base_components.h"

#include "events/events.h"
#include "memory/memory.h"
#include "render/imgui_game.h"
#include "render/render.h"
#include "tiles/tile_manager.h"
#include "tween/tween_manager.h"
#include "ui/ui_manager.h"

#include <heart/debug/assert.h>
#include <heart/deserialization/deserialization_file.h>
#include <heart/stl/unordered_map.h>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <heart/io/io_cmd_list.h>
#include <heart/io/io_cmd_queue.h>
#include <heart/sync/fence.h>

#include <entt/entt.hpp>

enum InputKey
{
	InputLeft = 1 << 0,
	InputRight = 1 << 1,
	InputUp = 1 << 2,
	InputDown = 1 << 3,
};

static entt::registry s_registry;

hrt::unordered_map<sf::Keyboard::Key, InputKey> s_keymap = {
	{sf::Keyboard::W, InputUp},
	{sf::Keyboard::Up, InputUp},
	{sf::Keyboard::A, InputLeft},
	{sf::Keyboard::Left, InputLeft},
	{sf::Keyboard::S, InputDown},
	{sf::Keyboard::Down, InputDown},
	{sf::Keyboard::D, InputRight},
	{sf::Keyboard::Right, InputRight}};

static UI::UIManager s_uiManager;
static TileManager s_tileManager;

static PlayerValues s_playerVals;

static bool sPlayerInputDown(sf::Event e)
{
	bool success = false;

	if (auto iter = s_keymap.find(e.key.code); iter != s_keymap.end())
	{
		s_registry.view<PlayerTag, InputStatusComponent>().each([iter, &success](auto playerEntity, auto& inputState) {
			inputState.flags |= iter->second;
			success = true;
		});
	}

	return success;
}

static bool sPlayerInputUp(sf::Event e)
{
	bool success = false;

	if (auto iter = s_keymap.find(e.key.code); iter != s_keymap.end())
	{
		s_registry.view<PlayerTag, InputStatusComponent>().each([iter, &success](auto playerEntity, auto& inputState) {
			inputState.flags &= ~iter->second;
			success = true;
		});
	}

	return success;
}

void InitializeGame()
{
	s_registry.on_destroy<DrawableComponent>().connect<&DrawableComponent::OnDestroy>();

	{
		IoCmdQueue queue;
		HeartFence fence;
		IoCmdList cmdList;

		uint64_t size = 0;

		// Load the player constants
		HeartGetFileSize("json/player_constants.json", size);
		hrt::vector<uint8_t> playerConstantsBuffer(size, 0);
		cmdList.BindIoFileDescriptor("json/player_constants.json");
		cmdList.BindIoTargetBuffer(IoCheckedTargetBuffer {playerConstantsBuffer.data(), playerConstantsBuffer.size()});
		cmdList.ReadEntire();
		cmdList.Signal(&fence, 1);

		// Load the background image
		HeartGetFileSize("textures/bg.png", size);
		hrt::vector<uint8_t> bgTextureBuffer(size, 0);
		cmdList.BindIoFileDescriptor("textures/bg.png");
		cmdList.BindIoTargetBuffer(IoCheckedTargetBuffer {bgTextureBuffer.data(), bgTextureBuffer.size()});
		cmdList.ReadEntire();
		cmdList.Signal(&fence, 2);

		// Load the tileset data (not actually used, just a test)
		HeartGetFileSize("json/tileset_list.json", size);
		hrt::vector<uint8_t> tilesetBuffer(size, 0);
		cmdList.BindIoFileDescriptor("json/tileset_list.json");
		cmdList.BindIoTargetBuffer(IoCheckedTargetBuffer {tilesetBuffer.data(), tilesetBuffer.size()});
		cmdList.ReadEntire();
		cmdList.Signal(&fence, 3);

		// Start loads
		queue.Submit(&cmdList);

		// Wait for the player constants
		fence.Wait(1);

		hrt::vector<uint8_t> playerTextureBuffer;
		{
			// Parse the constants
			rapidjson::Document jsonDoc;
			jsonDoc.Parse((const char*)playerConstantsBuffer.data(), playerConstantsBuffer.size());
			if (!jsonDoc.HasParseError())
			{
				HeartDeserializeObject(s_playerVals, jsonDoc);

				if (HeartGetFileSize(s_playerVals.texture.c_str(), size) && size)
				{
					playerTextureBuffer.resize(size);
					cmdList.BindIoFileDescriptor(s_playerVals.texture.c_str());
					cmdList.BindIoTargetBuffer(IoCheckedTargetBuffer {playerTextureBuffer.data(), playerTextureBuffer.size()});
					cmdList.ReadEntire();
					cmdList.Signal(&fence, 4);
					queue.Submit(&cmdList);
				}
			}
		}

		// Create the background
		fence.Wait(2);
		{
			auto bg = create_multi_component<TransformComponent, DrawableComponent>();
			auto& drawable = hrt::get<2>(bg);

			drawable.texture = new sf::Texture();
			drawable.texture->loadFromMemory(bgTextureBuffer.data(), bgTextureBuffer.size());

			drawable.sprite = new sf::Sprite(*drawable.texture);
			drawable.z = -10.0f;

			auto& tf = hrt::get<1>(bg);
			tf.position = sf::Vector2f(0.0f, -250.0f);
		}

		// Wait for the player texture if it existed, then create the player
		if (!playerTextureBuffer.empty())
		{
			fence.Wait(4);

			auto player = create_multi_component<PlayerTag, InputStatusComponent, TransformComponent, DrawableComponent>();
			auto& drawable = hrt::get<4>(player);

			drawable.texture = new sf::Texture();
			drawable.texture->loadFromMemory(playerTextureBuffer.data(), playerTextureBuffer.size());

			drawable.sprite = new sf::Sprite(*drawable.texture);
		}
	}

	// Create our origin marker
	{
		auto originMarker = create_multi_component<TransformComponent, DrawableComponent>();
		auto& drawable = hrt::get<2>(originMarker);

		sf::Image i;
		i.create(2, 2, sf::Color::Magenta);

		drawable.texture = new sf::Texture();
		drawable.texture->loadFromImage(i);

		auto rect = new sf::Sprite(*drawable.texture);
		drawable.sprite = rect;
	}

	// Load and create a UI button
	{
		s_uiManager.Initialize();
		s_uiManager.LoadPanel("blah");
	}

	// Load up the tilesets
	{
		s_tileManager.Initialize("json/tileset_list.json");
	}

	{
		auto handle = EventManager::Get().CreateHandler(sf::Event::KeyPressed);
		hrt::get<1>(handle).connect<sPlayerInputDown>();
	}

	{
		auto handle = EventManager::Get().CreateHandler(sf::Event::KeyReleased);
		hrt::get<1>(handle).connect<sPlayerInputUp>();
	}
}

entt::registry& GetRegistry()
{
	return s_registry;
}

UI::UIManager& GetUIManager()
{
	return s_uiManager;
}

void ShutdownGame()
{
	s_tileManager.Dispose();
	s_uiManager.Cleanup();
	s_registry.clear();
}

void RunGameTick(float deltaT)
{
	s_registry.view<PlayerTag, InputStatusComponent, TransformComponent>().each([=](auto p, auto& s, auto& t) {
		sf::Vector2f move(0.0f, 0.0f);

		if (s.flags & InputUp)
			move.y += 1.0f;
		if (s.flags & InputDown)
			move.y -= 1.0f;
		if (s.flags & InputLeft)
			move.x -= 1.0f;
		if (s.flags & InputRight)
			move.x += 1.0f;

		move *= s_playerVals.speed * deltaT;
		t.position += move;
	});

	s_registry.sort<DrawableComponent>([](const auto& lhs, const auto& rhs) { return lhs.z < rhs.z; });
}

void DrawGame(Renderer& r)
{
	auto camera = r.GetCameraRef().GetTransform();
	s_registry.view<DrawableComponent, TransformComponent>().each([&](auto entity, auto& draw, auto& transform) {
		// In "world" coordinates, 0,0 is the bottom left, so also offset the height of
		// the sprite in addition to the camera transform
		float height = draw.sprite->getGlobalBounds().height;
		draw.sprite->setPosition(camera.transformPoint(transform.position + sf::Vector2f(0.0f, height)));
		r.Draw(*draw.sprite);
	});

	s_tileManager.Render(r);
	s_uiManager.Render(r);

	Memory::DebugDisplay();
}
