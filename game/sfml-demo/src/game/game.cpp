#include "game.h"

#include "events/events.h"
#include "render/imgui_game.h"
#include "render/render.h"

#include <heart/debug/assert.h>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <entt/entt.hpp>

entt::registry s_registry;

enum InputKey
{
	InputLeft = 1 << 0,
	InputRight = 1 << 1,
	InputUp = 1 << 2,
	InputDown = 1 << 3,
};

struct InputStatus
{
	uint8_t flags = 0;
};

struct EnttTransformable
{
	sf::Vector2f position;
	sf::Vector2f rotation;
};

struct Drawable
{
	sf::Sprite* sprite = nullptr;
	sf::Texture* texture = nullptr;

	static void OnDestroy(entt::entity e)
	{
		auto& d = s_registry.get<Drawable>(e);
		delete d.sprite;
		d.sprite = nullptr;

		delete d.texture;
		d.texture = nullptr;
	}
};

struct PlayerTag
{
};

static bool sPlayerInputDown(sf::Event e)
{
	auto view = s_registry.view<PlayerTag, InputStatus>();
	HEART_ASSERT(view.size() == 1);

	entt::entity playerEntity = *view.begin();
	auto& inputState = s_registry.get<InputStatus>(playerEntity);

	if (e.key.code == sf::Keyboard::W || e.key.code == sf::Keyboard::Up)
	{
		inputState.flags |= InputUp;
		return true;
	}
	else if (e.key.code == sf::Keyboard::A || e.key.code == sf::Keyboard::Left)
	{
		inputState.flags |= InputLeft;
		return true;
	}
	else if (e.key.code == sf::Keyboard::S || e.key.code == sf::Keyboard::Down)
	{
		inputState.flags |= InputDown;
		return true;
	}
	else if (e.key.code == sf::Keyboard::D || e.key.code == sf::Keyboard::Right)
	{
		inputState.flags |= InputRight;
		return true;
	}

	return false;
}

static bool sPlayerInputUp(sf::Event e)
{
	auto view = s_registry.view<PlayerTag, InputStatus>();
	HEART_ASSERT(view.size() == 1);

	entt::entity playerEntity = *view.begin();
	auto& inputState = s_registry.get<InputStatus>(playerEntity);

	if (e.key.code == sf::Keyboard::W || e.key.code == sf::Keyboard::Up)
	{
		inputState.flags &= ~InputUp;
		return true;
	}
	else if (e.key.code == sf::Keyboard::A || e.key.code == sf::Keyboard::Left)
	{
		inputState.flags &= ~InputLeft;
		return true;
	}
	else if (e.key.code == sf::Keyboard::S || e.key.code == sf::Keyboard::Down)
	{
		inputState.flags &= ~InputDown;
		return true;
	}
	else if (e.key.code == sf::Keyboard::D || e.key.code == sf::Keyboard::Right)
	{
		inputState.flags &= ~InputRight;
		return true;
	}

	return false;
}

void InitializeGame()
{
	s_registry.on_destroy<Drawable>().connect<&Drawable::OnDestroy>();

	// Create our player
	{
		auto player = s_registry.create<PlayerTag, InputStatus, EnttTransformable, Drawable>();
		auto& drawable = std::get<4>(player);

		drawable.texture = new sf::Texture();
		HEART_CHECK(RenderUtils::LoadTextureFromFile(*drawable.texture, "textures/player.png"));

		drawable.sprite = new sf::Sprite(*drawable.texture);
	}

	// Create our origin marker
	{
		auto originMarker = s_registry.create<EnttTransformable, Drawable>();
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
		auto bg = s_registry.create<EnttTransformable, Drawable>();
		auto& drawable = std::get<2>(bg);

		drawable.texture = new sf::Texture();
		HEART_CHECK(RenderUtils::LoadTextureFromFile(*drawable.texture, "textures/bg.png"));

		drawable.sprite = new sf::Sprite(*drawable.texture);

		auto& tf = std::get<1>(bg);
		tf.position = sf::Vector2f(0.0f, -250.0f);
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

void ShutdownGame()
{
	s_registry.reset();
}

void RunGameTick(float deltaT)
{
	s_registry.view<PlayerTag, InputStatus, EnttTransformable>().each([=](auto p, InputStatus& s, EnttTransformable& t) {
		sf::Vector2f move(0.0f, 0.0f);

		if (s.flags & InputUp)
			move.y += 1.0f;
		if (s.flags & InputDown)
			move.y -= 1.0f;
		if (s.flags & InputLeft)
			move.x -= 1.0f;
		if (s.flags & InputRight)
			move.x += 1.0f;

		move *= 200.0f * deltaT;
		t.position += move;
	});
}

void DrawGame(Renderer& r)
{
	auto camera = r.GetCameraTransform();
	s_registry.view<EnttTransformable, Drawable>().each([&](auto entity, EnttTransformable& tranform, Drawable& draw) {
		// In "world" coordinates, 0,0 is the bottom left, so also offset the height of
		// the sprite in addition to the camera transform
		float height = draw.sprite->getGlobalBounds().height;
		draw.sprite->setPosition(camera.transformPoint(tranform.position + sf::Vector2f(0.0f, height)));
		r.Draw(*draw.sprite);
	});

#if IMGUI_ENABLED
	{
		bool open = true;
		auto player = *s_registry.view<PlayerTag, EnttTransformable>().begin();
		auto t = s_registry.get<EnttTransformable>(player);

		if (ImGui::Game::IsActive())
		{
			if (ImGui::Begin("x", &open,
					ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground |
						ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
						ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::SetCursorPos(ImVec2(25.0f, 25.0f));
				ImGui::Text("%f, %f", t.position.x, t.position.y);
			}

			ImGui::End();
		}
	}
#endif
}
