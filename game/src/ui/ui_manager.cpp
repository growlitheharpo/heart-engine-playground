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
		delete w;
	}

	m_widgets.clear();

	for (auto& fPair : m_loadedFonts)
	{
		delete fPair.second;
		fPair.second = nullptr;
	}

	m_loadedFonts.clear();
}

void UI::UIManager::LoadPanel(const char* panelName)
{
	UI::Button::ButtonData data;
	HeartDeserializeObjectFromFile(data, "json/button1.json");

	auto button = new UI::Button(data);
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

	auto result = new sf::Font(tmp);
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
