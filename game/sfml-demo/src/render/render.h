#pragma once

#include <heart/copy_move_semantics.h>

#include <SFML/System/Clock.hpp>

#include <SFML/Graphics/Transform.hpp>

namespace sf
{
	class RenderWindow;
	class Clock;
	class Event;
	class Drawable;
	class Texture;
}

class Renderer
{
private:
	sf::RenderWindow* window_ = nullptr;
	sf::Clock clock_;

	bool HandleResize(sf::Event& e);

public:
	Renderer() = default;
	DISABLE_COPY_SEMANTICS(Renderer);
	USE_DEFAULT_MOVE_SEMANTICS(Renderer);
	~Renderer();

	void Initialize();
	void Dispose();

	void RegisterEvents();

	void BeginFrame();

	void Draw(sf::Drawable&);

	void SubmitFrame();

	sf::Transform GetCameraTransform() const;

private:
	friend class EventManager;
	sf::RenderWindow* GetWindow()
	{
		return window_;
	}
};

namespace RenderUtils
{
	bool LoadTextureFromFile(sf::Texture& outTexture, const char* path);
}
