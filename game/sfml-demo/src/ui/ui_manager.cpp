#include "ui_manager.h"

#include "ui/button.h"
#include "ui/widget.h"

#include "events/events.h"

#include <heart/deserialization_file.h>

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
	for (auto w : widgets_)
	{
		w->Destroy();
		delete w;
	}

	widgets_.clear();
}

void UI::UIManager::LoadPanel(const char* panelName)
{
	UI::Button::ButtonData data;
	HeartDeserializeObjectFromFile(data, "json/button1.json");

	auto button = new UI::Button(data);
	button->Initialize();

	widgets_.push_back(button);
}

void UI::UIManager::Update()
{
	for (auto w : widgets_)
	{
		w->Update();
	}
}

void UI::UIManager::Render(Renderer& r)
{
	for (auto w : widgets_)
	{
		w->Draw(r);
	}
}
