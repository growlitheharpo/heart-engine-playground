#pragma once

#include <heart/copy_move_semantics.h>

#include <heart/stl/unordered_map.h>
#include <heart/stl/vector.h>

#include <heart/deserialization_fwd.h>

#include <entt/signal/delegate.hpp>

#include <SFML/Graphics.hpp>

class Renderer;

SERIALIZE_STRUCT()
struct MyDataTypeOther
{
	int x, w;
	int y;
	float value;
	SerializedString<32> wowza;
};

SERIALIZE_STRUCT()
struct SampleNestedObject
{
	int secondintval;
};

SERIALIZE_STRUCT()
struct SampleTargetType
{
	int intval;
	float floatval;
	SerializedString<72> string;

	SampleNestedObject nestedobject;
};

class EventManager
{
public:
	SERIALIZE_STRUCT()
	struct MyDataType
	{
		int x;
		int y;
		float value;

		SERIALIZE_AS_REF() SerializedString<64> wowza;
	};

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

public:
	EventFilterFunc& CreateHandler(sf::Event::EventType e);

	void Initialize(Renderer* r);
	void Dispose();

	void Process();
};
