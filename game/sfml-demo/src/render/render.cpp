#include "render.h"
#include "imgui_game.h"
#include "events/events.h"

#include <heart/debug/assert.h>
#include <heart/debug/imgui.h>

#include <heart/file.h>

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

bool Renderer::HandleResize(const sf::Event& e)
{
	// sf::FloatRect visibleArea(0, 0, float(e.size.width), float(e.size.height));
	// window_->setView(sf::View(visibleArea));
	// return true;
	return true;
}

void Renderer::RegisterEvents()
{
	auto eventHandle = EventManager::Get().CreateHandler(sf::Event::Resized);
	hrt::get<1>(eventHandle).connect<&Renderer::HandleResize>(*this);
}

void Renderer::BeginFrame()
{
	ImGui::Game::BeginRender();
	window_->clear();
}

void Renderer::Draw(sf::Drawable& d)
{
	/*
	auto height = float(window_->getSize().y);
	auto cameraTf = sf::Transform::Identity;
	cameraTf.scale(1.0f, -1.0f, 0.0f, 0.0f);
	cameraTf.translate(0.0f, -height);

	auto state = sf::RenderStates(cameraTf);*/
	auto state = sf::RenderStates();

	window_->draw(d, state);
}

void Renderer::SubmitFrame()
{
	ImGui::Game::SubmitRender(window_);
	window_->display();
}

sf::Transform Renderer::GetCameraTransform() const
{
	auto height = float(window_->getSize().y);
	auto cameraTf = sf::Transform::Identity;
	cameraTf.scale(1.0f, -1.0f, 0.0f, 0.0f);
	cameraTf.translate(0.0f, -height);
	return cameraTf;
}

bool RenderUtils::LoadTextureFromFile(sf::Texture& outTexture, const char* path)
{
	HeartFile file;
	if (!HeartOpenFile(file, path, HeartOpenFileMode::ReadExisting))
		return false;

	uint64_t size;
	if (!HeartGetFileSize(file, size))
		return false;

	hrt::vector<uint8_t> data;
	data.resize(size, 0);

	if (!HeartReadFile(file, data.data(), data.size(), size))
		return false;

	return outTexture.loadFromMemory(data.data(), data.size());
}
