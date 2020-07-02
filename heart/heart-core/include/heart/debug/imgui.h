#pragma once

#include <heart/config.h>

#if !defined(IMGUI_ENABLED)
#if HEART_STRICT_PERF
#define IMGUI_ENABLED 0
#else
#define IMGUI_ENABLED 1
#endif
#endif

#if IMGUI_ENABLED

#include <imgui-SFML.h>
#include <imgui.h>

#endif
