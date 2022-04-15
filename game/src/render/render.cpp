/* Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
#include "render.h"
#include "events/events.h"
#include "imgui_game.h"

#include <heart/debug/assert.h>
#include <heart/debug/imgui.h>

#include <heart/util/file_load.h>

#include <SFML/Graphics.hpp>

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
	extern bool MemoryImguiPanelActive;

	ToolType tools[] = {
		{"Tile Manager", &TileManagerImguiPanelActive},
		{"Memory", &MemoryImguiPanelActive},
	};

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
	if (m_window != nullptr)
		Dispose();
}

void Renderer::Initialize()
{
	m_window = new sf::RenderWindow(
		sf::VideoMode(1280, 720), "SFML works!", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
	HEART_ASSERT(m_window != nullptr, "Could not initialize window!");

	ImGui::Game::Init(m_window);
}

void Renderer::Dispose()
{
	m_window->close();
	delete m_window;
	m_window = nullptr;

	ImGui::Game::Shutdown();
}

bool Renderer::HandleResize(const sf::Event& e)
{
	// sf::FloatRect visibleArea(0, 0, float(e.size.width), float(e.size.height));
	// m_window->setView(sf::View(visibleArea));
	// return true;
	return true;
}

Renderer& Renderer::Get()
{
	static Renderer r;
	return r;
}

void Renderer::RegisterEvents()
{
	auto eventHandle = EventManager::Get().CreateHandler(sf::Event::Resized);
	std::get<1>(eventHandle).connect<&Renderer::HandleResize>(*this);
}

void Renderer::BeginFrame()
{
	m_window->clear();
	sDrawMenuPanel();
}

void Renderer::Draw(const sf::Drawable& d)
{
	auto state = sf::RenderStates();
	m_window->draw(d, state);
}

void Renderer::SubmitFrame()
{
	ImGui::Game::SubmitRender(m_window);
	m_window->display();
}

const sf::Window& Renderer::GetWindowRef() const
{
	HEART_ASSERT(m_window != nullptr, "Cannot get window reference before initiailization!");
	return *m_window;
}

sf::Vector2f Renderer::GetScreenSize() const
{
	if (m_window == nullptr)
		return {};

	auto pixelSize = m_window->getSize();
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
	auto& window = Renderer::Get().GetWindowRef();
	return sf::Mouse::getPosition(window);
}
