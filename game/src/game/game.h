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

template <typename... T>
std::tuple<entt::entity, T&...> create_multi_component()
{
	entt::entity e = GetRegistry().create();
	return std::make_tuple(e, std::ref(GetRegistry().assign<T>(e)) ...);
}

template <typename... T>
std::tuple<T&...> assign_multi_component(entt::entity e)
{
	return std::make_tuple(e, std::ref(GetRegistry().assign<T>(e))...);
}

UI::UIManager& GetUIManager();

void ShutdownGame();

void DrawGame(Renderer& r);
