#include "game.h"
#include "base_components.h"

#include "events/events.h"
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

#include <entt/entt.hpp>

enum InputKey
{
	InputLeft = 1 << 0,
	InputRight = 1 << 1,
	InputUp = 1 << 2,
	InputDown = 1 << 3,
};

static entt::registry s_registry;

hrt::unordered_map<sf::Keyboard::Key, InputKey> s_keymap = {{sf::Keyboard::W, InputUp}, {sf::Keyboard::Up, InputUp},
	{sf::Keyboard::A, InputLeft}, {sf::Keyboard::Left, InputLeft}, {sf::Keyboard::S, InputDown},
	{sf::Keyboard::Down, InputDown}, {sf::Keyboard::D, InputRight}, {sf::Keyboard::Right, InputRight}};

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

	// Create our player
	{
		HeartDeserializeObjectFromFile(s_playerVals, "json/player_constants.json");

		auto player = create_multi_component<PlayerTag, InputStatusComponent, TransformComponent, DrawableComponent>();
		auto& drawable = std::get<4>(player);

		drawable.texture = new sf::Texture();
		HEART_CHECK(RenderUtils::LoadTextureFromFile(*drawable.texture, s_playerVals.texture.c_str()));

		drawable.sprite = new sf::Sprite(*drawable.texture);
	}

	// Create our origin marker
	{
		auto originMarker = create_multi_component<TransformComponent, DrawableComponent>();
		auto& drawable = std::get<2>(originMarker);

		sf::Image i;
		i.create(2, 2, sf::Color::Magenta);

		drawable.texture = new sf::Texture();
		drawable.texture->loadFromImage(i);

		auto rect = new sf::Sprite(*drawable.texture);
		drawable.sprite = rect;
	}

	// Create the background
	{
		auto bg = create_multi_component<TransformComponent, DrawableComponent>();
		auto& drawable = std::get<2>(bg);

		drawable.texture = new sf::Texture();
		HEART_CHECK(RenderUtils::LoadTextureFromFile(*drawable.texture, "textures/bg.png"));

		drawable.sprite = new sf::Sprite(*drawable.texture);
		drawable.z = -10.0f;

		auto& tf = std::get<1>(bg);
		tf.position = sf::Vector2f(0.0f, -250.0f);
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
}
