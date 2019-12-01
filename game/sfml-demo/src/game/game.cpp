#include "game.h"
#include "base_components.h"

#include "events/events.h"
#include "render/imgui_game.h"
#include "render/render.h"
#include "tween/tween_manager.h"
#include "ui/ui_manager.h"

#include <heart/debug/assert.h>
#include <heart/deserialization_file.h>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <entt/entt.hpp>

entt::registry s_registry;

UI::UIManager s_uiManager;

TweenManager::Wrapper alphaTween(tweeny::from(0.0f));
TweenManager::Wrapper sizeTween(tweeny::from(sf::Vector2f()));
TweenManager::Wrapper posTween(tweeny::from(sf::Vector2f()));
sf::RectangleShape rectShape;

enum InputKey
{
	InputLeft = 1 << 0,
	InputRight = 1 << 1,
	InputUp = 1 << 2,
	InputDown = 1 << 3,
};

static bool sPlayerInputDown(sf::Event e)
{
	bool success = false;
	s_registry.view<PlayerTag, InputStatusComponent>().each([e, &success](auto playerEntity, auto& inputState) {
		if (e.key.code == sf::Keyboard::W || e.key.code == sf::Keyboard::Up)
		{
			inputState.flags |= InputUp;
			success = true;
			return;
		}
		else if (e.key.code == sf::Keyboard::A || e.key.code == sf::Keyboard::Left)
		{
			inputState.flags |= InputLeft;
			success = true;
			return;
		}
		else if (e.key.code == sf::Keyboard::S || e.key.code == sf::Keyboard::Down)
		{
			inputState.flags |= InputDown;
			success = true;
			return;
		}
		else if (e.key.code == sf::Keyboard::D || e.key.code == sf::Keyboard::Right)
		{
			inputState.flags |= InputRight;
			success = true;
			return;
		}
	});

	return success;
}

static bool sPlayerInputUp(sf::Event e)
{
	bool success = false;

	s_registry.view<PlayerTag, InputStatusComponent>().each([e, &success](auto playerEntity, auto& inputState) {
		if (e.key.code == sf::Keyboard::W || e.key.code == sf::Keyboard::Up)
		{
			inputState.flags &= ~InputUp;
			success = true;
			return;
		}
		else if (e.key.code == sf::Keyboard::A || e.key.code == sf::Keyboard::Left)
		{
			inputState.flags &= ~InputLeft;
			success = true;
			return;
		}
		else if (e.key.code == sf::Keyboard::S || e.key.code == sf::Keyboard::Down)
		{
			inputState.flags &= ~InputDown;
			success = true;
			return;
		}
		else if (e.key.code == sf::Keyboard::D || e.key.code == sf::Keyboard::Right)
		{
			inputState.flags &= ~InputRight;
			success = true;
			return;
		}
	});

	return success;
}

void InitializeGame()
{
	s_registry.on_destroy<DrawableComponent>().connect<&DrawableComponent::OnDestroy>();

	// Create our player
	{
		auto player = s_registry.create<PlayerTag, InputStatusComponent, TransformableComponent, DrawableComponent>();
		auto& drawable = std::get<4>(player);

		drawable.texture = new sf::Texture();
		HEART_CHECK(RenderUtils::LoadTextureFromFile(*drawable.texture, "textures/player.png"));

		drawable.sprite = new sf::Sprite(*drawable.texture);
	}

	// Create our origin marker
	{
		auto originMarker = s_registry.create<TransformableComponent, DrawableComponent>();
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
		auto bg = s_registry.create<TransformableComponent, DrawableComponent>();
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

	{
		auto handle = EventManager::Get().CreateHandler(sf::Event::KeyPressed);
		hrt::get<1>(handle).connect<sPlayerInputDown>();
	}

	{
		auto handle = EventManager::Get().CreateHandler(sf::Event::KeyReleased);
		hrt::get<1>(handle).connect<sPlayerInputUp>();
	}

	{
		auto duration = sf::seconds(4.0f).asMilliseconds();
		alphaTween = tweeny::from(0.1f)
						 .to(1.0f)
						 .during(duration)
						 .via(tweeny::easing::linear)
						 .to(0.0f)
						 .during(sf::seconds(1.0f).asMilliseconds())
						 .via(tweeny::easing::linear);
		sizeTween = tweeny::from(sf::Vector2f())
						.to(Renderer::Get().GetScreenSize())
						.during(duration)
						.via(tweeny::easing::cubicInOut);
		posTween = tweeny::from(Renderer::Get().GetScreenSize() / 2.0f)
					   .to(sf::Vector2f())
					   .during(duration)
					   .via(tweeny::easing::cubicInOut);

		TweenManager::RegisterTween(alphaTween);
		TweenManager::RegisterTween(sizeTween);
		TweenManager::RegisterTween(posTween);
	}
}

entt::registry* GetRegistry()
{
	return &s_registry;
}

void ShutdownGame()
{
	s_uiManager.Cleanup();
	s_registry.reset();
}

void RunGameTick(float deltaT)
{
	s_registry.view<PlayerTag, InputStatusComponent, TransformableComponent>().each([=](auto p, auto& s, auto& t) {
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

	auto color = sf::Color(155, 0, 255, int(float(255) * alphaTween.impl.peek()));
	rectShape.setFillColor(color);
	rectShape.setPosition(posTween.impl.peek());
	rectShape.setSize(sizeTween.impl.peek());

	s_registry.sort<DrawableComponent>([](const auto& lhs, const auto& rhs) { return lhs.z < rhs.z; });
}

void DrawGame(Renderer& r)
{
	auto camera = r.GetCameraTransform();
	s_registry.view<DrawableComponent, TransformableComponent>().each([&](auto entity, auto& draw, auto& transform) {
		// In "world" coordinates, 0,0 is the bottom left, so also offset the height of
		// the sprite in addition to the camera transform
		float height = draw.sprite->getGlobalBounds().height;
		draw.sprite->setPosition(camera.transformPoint(transform.position + sf::Vector2f(0.0f, height)));
		r.Draw(*draw.sprite);
	});

	s_uiManager.Render(r);

	r.Draw(rectShape);

#if IMGUI_ENABLED
	{
		bool open = true;
		auto player = s_registry.view<PlayerTag, TransformableComponent>().begin();
		auto t = s_registry.get<TransformableComponent>(*player);

		if (ImGui::Game::IsActive())
		{
			if (ImGui::Begin("x", &open,
					ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar |
						ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::SetCursorPos(ImVec2(25.0f, 25.0f));
				ImGui::Text("%f, %f", t.position.x, t.position.y);
			}

			ImGui::End();
		}
	}
#endif
}
