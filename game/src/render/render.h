#pragma once

#include <render/camera.h>

#include <heart/copy_move_semantics.h>
#include <heart/debug/assert.h>

#include <SFML/System/Clock.hpp>

#include <SFML/Graphics/Transform.hpp>

namespace sf
{
	class Window;
	class RenderWindow;
	class Clock;
	class Event;
	class Drawable;
	class Texture;
}

class Renderer
{
private:
	sf::RenderWindow* m_window = nullptr;
	sf::Clock m_clock;
	Camera m_camera;

	bool HandleResize(const sf::Event& e);

	Renderer() = default;

public:
	static Renderer& Get(); // bleck :(

	DISABLE_COPY_SEMANTICS(Renderer);
	DISABLE_MOVE_SEMANTICS(Renderer);

	~Renderer();

	void Initialize();
	void Dispose();

	void RegisterEvents();

	void BeginFrame();

	void Draw(const sf::Drawable&);

	void SubmitFrame();

	sf::Vector2f GetScreenSize() const;

	const sf::Window& GetWindowRef() const;

	Camera& GetCameraRef()
	{
		return m_camera;
	}

private:
	friend class EventManager;
	sf::RenderWindow* GetWindow()
	{
		return m_window;
	}
};

namespace RenderUtils
{
	bool LoadTextureFromFile(sf::Texture& outTexture, const char* path);
	sf::Vector2i GetMousePosition();
}
