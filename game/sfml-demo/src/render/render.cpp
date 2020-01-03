#include "render.h"
#include "events/events.h"
#include "imgui_game.h"

#include <heart/debug/assert.h>
#include <heart/debug/imgui.h>

#include <heart/util/file_load.h>

#include <SFML/Graphics.hpp>

static Renderer* s_globalRenderer = nullptr;

static void sDrawMenuPanel()
{
	if (!ImGui::Game::IsActive())
		return;

#if IMGUI_ENABLED
	struct ToolType
	{
		const char* name;
		bool* toggle;
	};

	extern bool TileManagerImguiPanelActive;

	ToolType tools[] = {"Tile Manager", &TileManagerImguiPanelActive};

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Tools"))
		{
			for (auto& t : tools)
			{
				if (ImGui::MenuItem(t.name))
				{
					*t.toggle = !*t.toggle;
				}
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
#endif
}

Renderer::~Renderer()
{
	if (window_ != nullptr)
		Dispose();
}

void Renderer::Initialize()
{
	HEART_ASSERT(s_globalRenderer == nullptr, "Cannot have more than one renderer initialized!");
	s_globalRenderer = this;

	window_ = new sf::RenderWindow(
		sf::VideoMode(1280, 720), "SFML works!", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
	HEART_ASSERT(window_ != nullptr, "Could not initialize window!");

	ImGui::Game::Init(window_);
}

void Renderer::Dispose()
{
	s_globalRenderer = nullptr;

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

Renderer& Renderer::Get()
{
	HEART_ASSERT(s_globalRenderer != nullptr);
	return *s_globalRenderer;
}

void Renderer::RegisterEvents()
{
	auto eventHandle = EventManager::Get().CreateHandler(sf::Event::Resized);
	hrt::get<1>(eventHandle).connect<&Renderer::HandleResize>(*this);
}

void Renderer::BeginFrame()
{
	window_->clear();
	sDrawMenuPanel();
}

void Renderer::Draw(sf::Drawable& d)
{
	auto state = sf::RenderStates();
	window_->draw(d, state);
}

void Renderer::SubmitFrame()
{
	ImGui::Game::SubmitRender(window_);
	window_->display();
}

const sf::Window& Renderer::GetWindowRef() const
{
	HEART_ASSERT(window_ != nullptr, "Cannot get window reference before initiailization!");
	return *window_;
}

sf::Vector2f Renderer::GetScreenSize() const
{
	if (window_ == nullptr)
		return {};

	auto pixelSize = window_->getSize();
	return sf::Vector2f(float(pixelSize.x), float(pixelSize.y));
}

bool RenderUtils::LoadTextureFromFile(sf::Texture& outTexture, const char* path)
{
	hrt::vector<uint8_t> data = HeartUtilLoadExistingFile(path);
	if (data.empty())
		return false;

	return outTexture.loadFromMemory(data.data(), data.size());
}

sf::Vector2i RenderUtils::GetMousePosition()
{
	// TODO: More safety here!
	auto& window = s_globalRenderer->GetWindowRef();
	return sf::Mouse::getPosition(window);
}
