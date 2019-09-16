#pragma once

#include <heart/copy_move_semantics.h>

#include <heart/stl/vector.h>
#include <heart/stl/unordered_map.h>
#include <heart/stl/functional.h>

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

	using EventFilterFunc = hrt::function<bool(sf::Event)>;

private:
	hrt::unordered_map<sf::Event::EventType, hrt::vector<EventFilterFunc>> event_handlers_;

public:
	void RegisterHandler(sf::Event::EventType e, EventFilterFunc target);

	void Initialize(Renderer* r);
	void Dispose();

	void Process();
};
