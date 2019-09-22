#pragma once

class Renderer;

void InitializeGame();

void RunGameTick(float deltaT);

void ShutdownGame();

void DrawGame(Renderer& r);
