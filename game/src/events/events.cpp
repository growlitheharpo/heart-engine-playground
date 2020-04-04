#include "events.h"
#include "render/imgui_game.h"
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

void EventManager::ProcessEvent(sf::Event e)
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

hrt::tuple<EventManager::EventFuncHandle, EventManager::EventFilterFunc&> EventManager::CreateHandler(
	sf::Event::EventType e)
{
	auto& vec = event_handlers_[e];
	auto& func = vec.emplace_back();
	auto handle = EventFuncHandle(e, int32_t(vec.size() - 1));

	return hrt::tuple<EventManager::EventFuncHandle, EventManager::EventFilterFunc&>(handle, std::ref(func));
}

bool EventManager::RemoveHandler(EventFuncHandle id)
{
	if (id.type >= sf::Event::Count)
		return false;

	auto& vec = event_handlers_[id.type];
	if (id.idx >= vec.size())
		return false;

	// TODO: ideally, we'd like to actually remove the handler from the list to clean up the memory it's holding...
	// luckily, that shouldn't require any API changes since the handle is totally opaque to the caller
	vec[id.idx].reset();
	return true;
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

void EventManager::ManuallyIssueEvent(sf::Event e)
{
	ProcessEvent(e);
}

void EventManager::Process()
{
	auto window = renderer_->GetWindow();

	sf::Event e;
	while (window->pollEvent(e))
	{
		ProcessEvent(e);
	}

	ImGui::Game::Tick(window);
}
