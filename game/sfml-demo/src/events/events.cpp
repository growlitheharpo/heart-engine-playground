#include "events.h"
#include "render/render.h"

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

void EventManager::Initialize(Renderer* r)
{
	renderer_ = r;
}

void EventManager::Dispose()
{
	renderer_ = nullptr;
	event_handlers_.clear();
}

void EventManager::RegisterHandler(sf::Event::EventType e, EventManager::EventFilterFunc target)
{
	event_handlers_[e].push_back(target);
}

void EventManager::Process()
{
	auto& window = *renderer_->GetWindow();

	sf::Event e;
	while (window.pollEvent(e))
	{
		auto targets = event_handlers_.find(e.type);
		if (targets != event_handlers_.end() && !targets->second.empty())
		{
			for (auto& handler : targets->second)
			{
				if (handler(e))
					break;
			}
		}

#if IMGUI_ENABLED
		ImGui::SFML::ProcessEvent(e);
#endif
	}

#if IMGUI_ENABLED
	static sf::Clock imGuiClock;
	ImGui::SFML::Update(window, imGuiClock.restart());
#endif
}
