/* Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
#include "ui_manager.h"

#include "ui/button.h"
#include "ui/widget.h"

#include "events/events.h"

#include <heart/deserialization/deserialization_file.h>
#include <heart/util/file_load.h>

bool UI::GlobalButtonFunctionality::CloseGame()
{
	sf::Event e;
	e.type = sf::Event::Closed;

	EventManager::Get().ManuallyIssueEvent(e);
	return true;
}

bool UI::GlobalButtonFunctionality::TogglePause()
{
	return true;
}

bool UI::GlobalButtonFunctionality::ResetPlayer()
{
	return true;
}

void UI::UIManager::Initialize()
{
}

void UI::UIManager::Cleanup()
{
	for (auto w : m_widgets)
	{
		w->Destroy();
		Memory::Delete<Memory::UILongAllocator>(w);
	}

	m_widgets.clear();

	for (auto& fPair : m_loadedFonts)
	{
		Memory::Delete<Memory::UILongAllocator>(fPair.second);
	}

	m_loadedFonts.clear();
}

void UI::UIManager::LoadPanel(const char* panelName)
{
	UI::Button::ButtonData data;
	HeartDeserializeObjectFromFile(data, "json/button1.json");

	auto button = Memory::New<Memory::UILongAllocator, UI::Button>(data);
	button->Initialize();

	m_widgets.push_back(button);
}

sf::Font* UI::UIManager::FindOrLoadFont(const char* fontName)
{
	auto iter = m_loadedFonts.find(fontName);
	if (iter != m_loadedFonts.end())
	{
		return iter->second;
	}

	auto fileData = HeartUtilLoadExistingFile(fontName);
	sf::Font tmp;
	if (!tmp.loadFromMemory(fileData.data(), fileData.size()))
		return nullptr;

	auto result = Memory::New<Memory::UILongAllocator, sf::Font>(tmp);
	m_loadedFonts[fontName] = result;
	return result;
}

void UI::UIManager::Update()
{
	for (auto w : m_widgets)
	{
		w->Update();
	}
}

void UI::UIManager::Render(Renderer& r)
{
	for (auto w : m_widgets)
	{
		w->Draw(r);
	}
}
