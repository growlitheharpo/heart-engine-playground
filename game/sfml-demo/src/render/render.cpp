#include "render.h"
#include "imgui_game.h"
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
	window_ = new sf::RenderWindow(
		sf::VideoMode(1280, 720), "SFML works!", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
	HEART_ASSERT(window_ != nullptr, "Could not initialize window!");

	ImGui::Game::Init(window_);
}

void Renderer::Dispose()
{
	window_->close();
	delete window_;
	window_ = nullptr;

	ImGui::Game::Shutdown();
}

bool Renderer::HandleResize(sf::Event& e)
{
	sf::FloatRect visibleArea(0, 0, float(e.size.width), float(e.size.height));
	window_->setView(sf::View(visibleArea));
	return true;
}

void Renderer::RegisterEvents()
{
	EventManager::Get().CreateHandler(sf::Event::Resized).connect<&Renderer::HandleResize>(*this);
}

void Renderer::BeginFrame()
{
	ImGui::Game::BeginRender();
	window_->clear();
}

void Renderer::Draw(sf::Drawable& d)
{
	window_->draw(d);
}

void Renderer::SubmitFrame()
{
	ImGui::Game::SubmitRender(window_);
	window_->display();
}
