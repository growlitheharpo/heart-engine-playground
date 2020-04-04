#pragma once

#include <entt/fwd.hpp>

#include <heart/codegen/codegen.h>

SERIALIZE_STRUCT()
struct PlayerValues
{
	float speed = 1.0f;
	SERIALIZE_AS_REF() SerializedDataPath texture;
};

class Renderer;

namespace UI
{
	class UIManager;
}

void InitializeGame();

void RunGameTick(float deltaT);

entt::registry& GetRegistry();

template <typename T>
struct multi_component_return_type
{
	using type = T&;
};

template <auto Value>
struct multi_component_return_type<entt::tag<Value>>
{
	using type = entt::tag<Value>;
};

template <typename... T>
auto create_multi_component()
{
	entt::entity e = GetRegistry().create();
	return std::tuple<entt::entity, multi_component_return_type<T>::type...>(e, GetRegistry().assign<T>(e)...);
}

template <typename... T>
auto assign_multi_component(entt::entity e)
{
	return std::tuple<multi_component_return_type<T>::type...>(GetRegistry().assign<T>(e)...);
}

UI::UIManager& GetUIManager();

void ShutdownGame();

void DrawGame(Renderer& r);
