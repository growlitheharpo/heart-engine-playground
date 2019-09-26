#include "events.h"
#include "render/render.h"
#include "render/imgui_game.h"

#include <heart/debug/imgui.h>

EventManager& EventManager::Get()
{
	static EventManager e;
	return e;
}

EventManager::~EventManager()
{
	if (renderer_ != nullptr)
		Dispose();
}

EventManager::EventFilterFunc& EventManager::CreateHandler(sf::Event::EventType e)
{
	return event_handlers_[e].emplace_back();
}

void EventManager::Initialize(Renderer* r)
{
	renderer_ = r;
}

void EventManager::Dispose()
{
	renderer_ = nullptr;
	event_handlers_.clear();
}

void EventManager::Process()
{
	auto window = renderer_->GetWindow();

	sf::Event e;
	while (window->pollEvent(e))
	{
		auto targets = event_handlers_.find(e.type);
		if (targets != event_handlers_.end() && !targets->second.empty())
		{
			// We iterate over the list backwards - i.e. the most recent one to register
			// gets first dibs on handling the event
			for (auto i = targets->second.rbegin(); i != targets->second.rend(); ++i)
			{
				auto& handler = *i;
				if (handler && handler(e))
					break;
			}
		}

		ImGui::Game::ProcessEvent(e);
	}

	ImGui::Game::Tick(window);
}
