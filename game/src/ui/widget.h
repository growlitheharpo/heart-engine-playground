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
		TransformUI transform_ = {};
		hrt::weak_ptr<Widget> parent_ = {};

	public:
		void SetParent(hrt::shared_ptr<Widget> parent)
		{
			parent_ = parent;
		}

		inline hrt::shared_ptr<Widget> GetParent() const
		{
			return parent_.lock();
		}

		inline sf::Vector2f GetLocalPosition() const
		{
			return transform_.position;
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
			transform_.position.x = x;
			transform_.dirty = true;
		}

		inline void SetPositionY(float y)
		{
			transform_.position.y = y;
			transform_.dirty = true;
		}

		virtual void Initialize() = 0;
		virtual void Destroy() = 0;
		virtual void Update() = 0;
		virtual void Draw(Renderer& r) const = 0;
	};
}
