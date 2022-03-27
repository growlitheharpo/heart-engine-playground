#pragma once

#include "memory/memory.h"

#include <heart/copy_move_semantics.h>

#include <heart/stl/unordered_map.h>
#include <heart/stl/utility.h>
#include <heart/stl/vector.h>

#include <entt/signal/delegate.hpp>

#include <SFML/Graphics.hpp>

class Renderer;

class EventManager
{
public:
	using EventFilterFunc = entt::delegate<bool(sf::Event)>;

	struct EventFuncHandle
	{
	private:
		sf::Event::EventType type = sf::Event::Count;
		uint32_t idx = INT32_MAX;

		EventFuncHandle(sf::Event::EventType e, uint32_t i) :
			type(e), idx(i)
		{
		}

	public:
		friend class EventManager;
		EventFuncHandle() = default;

		USE_DEFAULT_COPY_SEMANTICS(EventFuncHandle);
		USE_DEFAULT_MOVE_SEMANTICS(EventFuncHandle);
	};

private:
	EventManager() = default;
	Renderer* m_rendererRef = nullptr;

public:
	~EventManager();
	DISABLE_COPY_SEMANTICS(EventManager);
	DISABLE_MOVE_SEMANTICS(EventManager);

	static EventManager& Get();

	void ProcessAndDispatch();

private:
	hrt::unordered_map_a<
		sf::Event::EventType,
		hrt::vector_a<EventFilterFunc, Memory::EventsLongAllocator>,
		Memory::EventsLongAllocator>
		event_handlers_;
	void ProcessEvent(sf::Event e);

public:
	std::tuple<EventFuncHandle, EventFilterFunc&> CreateHandler(sf::Event::EventType e);
	bool RemoveHandler(EventFuncHandle id);

	void Initialize(Renderer* r);
	void Dispose();

	void ManuallyIssueEvent(sf::Event e);
	void Process();
};
