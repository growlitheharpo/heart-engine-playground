/* Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
#pragma once

#include <entt/core/type_traits.hpp>
#include <entt/fwd.hpp>

#include <heart/stl/utility.h>

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

UI::UIManager& GetUIManager();

void ShutdownGame();

void DrawGame(Renderer& r);
