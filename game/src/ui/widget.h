/* Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
#pragma once

#include <heart/stl/memory.h>

#include <SFML/System.hpp>

class Renderer;

namespace UI
{
	struct TransformUI
	{
		sf::Vector2f position = {};
		bool dirty = false;
	};

	class Widget
	{
	private:
		TransformUI m_transform = {};
		hrt::weak_ptr<Widget> m_parent = {};

	public:
		void SetParent(hrt::shared_ptr<Widget> parent)
		{
			m_parent = parent;
		}

		inline hrt::shared_ptr<Widget> GetParent() const
		{
			return m_parent.lock();
		}

		inline sf::Vector2f GetLocalPosition() const
		{
			return m_transform.position;
		}

		inline sf::Vector2f GetAbsolutePosition() const
		{
			auto parent = GetParent();
			auto offset = sf::Vector2f();

			if (parent)
			{
				offset = parent->GetAbsolutePosition();
			}

			return GetLocalPosition() + offset;
		}

		inline void SetPosition(sf::Vector2f p)
		{
			SetPosition(p.x, p.y);
		}

		inline void SetPosition(float x, float y)
		{
			SetPositionX(x);
			SetPositionY(y);
		}

		inline void SetPositionX(float x)
		{
			m_transform.position.x = x;
			m_transform.dirty = true;
		}

		inline void SetPositionY(float y)
		{
			m_transform.position.y = y;
			m_transform.dirty = true;
		}

		virtual void Initialize() = 0;
		virtual void Destroy() = 0;
		virtual void Update() = 0;
		virtual void Draw(Renderer& r) const = 0;
	};
}
