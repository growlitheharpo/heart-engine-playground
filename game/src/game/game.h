#pragma once

#include <entt/core/type_traits.hpp>
#include <entt/fwd.hpp>

#include <heart/codegen/codegen.h>

SERIALIZE_STRUCT()
struct PlayerValues
{
	float speed = 1.0f;

	SERIALIZE_AS_REF()
	SerializedDataPath texture;
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

	static type emplace_wrapper(entt::entity e)
	{
		return GetRegistry().emplace<T>(e);
	}
};

template <auto Value>
struct multi_component_return_type<entt::tag<Value>>
{
	using type = entt::tag<Value>;

	static type emplace_wrapper(entt::entity e)
	{
		GetRegistry().emplace<type>(e);
		return type();
	}
};

template <typename... T>
auto create_multi_component()
{
	entt::entity e = GetRegistry().create();
	return hrt::tuple<entt::entity, multi_component_return_type<T>::type...>(e, multi_component_return_type<T>::emplace_wrapper(e)...);
}

template <typename... T>
auto assign_multi_component(entt::entity e)
{
	return hrt::tuple<multi_component_return_type<T>::type...>(multi_component_return_type<T>::emplace_wrapper(e)...);
}

UI::UIManager& GetUIManager();

void ShutdownGame();

void DrawGame(Renderer& r);
