#pragma once

#include <entt/fwd.hpp>

#include <heart/deserialization_fwd.h>

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

entt::registry* GetRegistry();

UI::UIManager& GetUIManager();

void ShutdownGame();

void DrawGame(Renderer& r);
