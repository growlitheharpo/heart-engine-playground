/* Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
#include "tween_manager.h"

#include <heart/stl/vector.h>

static hrt::vector<TweenManager::BaseWrapper*> s_liveTweens;

void TweenManager::RegisterTween(BaseWrapper* t)
{
	s_liveTweens.push_back(t);
}

void TweenManager::UnregisterTween(BaseWrapper* t)
{
	auto iter = std::find(s_liveTweens.begin(), s_liveTweens.end(), t);
	if (iter != s_liveTweens.end())
	{
		*iter = s_liveTweens.back();
		s_liveTweens.pop_back();
	}
}

void TweenManager::Tick(uint32_t deltaMs)
{
	for (auto ptr : s_liveTweens)
	{
		ptr->Step(deltaMs);
	}
}
