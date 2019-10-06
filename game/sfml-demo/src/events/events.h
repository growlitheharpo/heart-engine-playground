#pragma once

#include <heart/copy_move_semantics.h>

#include <heart/stl/unordered_map.h>
#include <heart/stl/vector.h>

#include <entt/signal/delegate.hpp>

#include <SFML/Graphics.hpp>

class Renderer;

class EventManager
{
private:
	EventManager() = default;
	Renderer* renderer_ = nullptr;

public:
	~EventManager();
	DISABLE_COPY_SEMANTICS(EventManager);
	DISABLE_MOVE_SEMANTICS(EventManager);

	static EventManager& Get();

	void ProcessAndDispatch();

	using EventFilterFunc = entt::delegate<bool(sf::Event)>;

private:
	hrt::unordered_map<sf::Event::EventType, hrt::vector<EventFilterFunc>> event_handlers_;
	void ProcessEvent(sf::Event e);

public:
	EventFilterFunc& CreateHandler(sf::Event::EventType e);

	void Initialize(Renderer* r);
	void Dispose();

	void ManuallyIssueEvent(sf::Event e);
	void Process();
};
