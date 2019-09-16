#include "render.h"
#include "events/events.h"

#include <heart/debug/assert.h>
#include <heart/debug/imgui.h>

#include <SFML/Graphics.hpp>

Renderer::~Renderer()
{
	if (window_ != nullptr)
		Dispose();
}

void Renderer::Initialize()
{
	window_ = new sf::RenderWindow(sf::VideoMode(1280, 720), "SFML works!", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
	HEART_ASSERT(window_ != nullptr, "Could not initialize window!");

#if IMGUI_ENABLED
	ImGui::SFML::Init(*window_);
#endif
}

void Renderer::Dispose()
{
	window_->close();
	delete window_;
	window_ = nullptr;

#if IMGUI_ENABLED
	ImGui::SFML::Shutdown();
#endif
}

bool Renderer::HandleResize(sf::Event& e)
{
	sf::FloatRect visibleArea(0, 0, float(e.size.width), float(e.size.height));
	window_->setView(sf::View(visibleArea));
	return true;
}

void Renderer::RegisterEvents()
{
	EventManager::Get().RegisterHandler(sf::Event::Resized, hrt::bind(&Renderer::HandleResize, this, hrt::placeholders::_1));
}

void Renderer::BeginFrame()
{
#if IMGUI_ENABLED
	ImGui::EndFrame();
#endif

	window_->clear();
}

void Renderer::Draw(sf::Drawable& d)
{
	window_->draw(d);
}

void Renderer::SubmitFrame()
{
#if IMGUI_ENABLED
	ImGui::SFML::Render(*window_);
#endif

	window_->display();
}
