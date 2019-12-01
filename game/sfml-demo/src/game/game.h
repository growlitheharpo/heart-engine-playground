#pragma once

#include <entt/fwd.hpp>

class Renderer;

void InitializeGame();

void RunGameTick(float deltaT);

entt::registry* GetRegistry();

void ShutdownGame();

void DrawGame(Renderer& r);
