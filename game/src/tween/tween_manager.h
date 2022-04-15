/* Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

#include <heart/types.h>

#pragma warning(push)
#pragma warning(disable : 4244)
#include <tweeny.h>
#pragma warning(pop)

class TweenManager
{
public:
	struct BaseWrapper
	{
		virtual void Step(uint32_t dt) = 0;
		virtual bool Done() const = 0;
	};

	template <typename T>
	struct Wrapper : public BaseWrapper
	{
		T impl;

		Wrapper(T r)
		{
			impl = std::move(r);
		}

		Wrapper& operator=(T r)
		{
			impl = std::move(r);
			return *this;
		}

		void Step(uint32_t dt) override
		{
			impl.step(dt);
		}

		bool Done() const override
		{
			return impl.progress() >= 1.0f;
		}
	};

	static void RegisterTween(BaseWrapper* t);

	static void UnregisterTween(BaseWrapper* t);

	template <typename T>
	static void RegisterTween(Wrapper<T>& t)
	{
		RegisterTween(static_cast<BaseWrapper*>(&t));
	}

	template <typename T>
	static void UnregisterTween(Wrapper<T>& t)
	{
		UnregisterTween(static_cast<BaseWrapper*>(&t));
	}

	static void Tick(uint32_t deltaMs);
};

inline sf::Vector2f operator/(const sf::Vector2f& left, int right)
{
	auto copy = left;
	copy.x /= float(right);
	copy.y /= float(right);
	return copy;
}
