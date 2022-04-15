/* Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
#include "text.h"

#include "game/game.h"
#include "render/render.h"
#include "ui/ui_manager.h"

#include <sfml/Graphics/Text.hpp>

UI::Text::Text(TextData data)
{
	sf::Font* font = GetUIManager().FindOrLoadFont(data.fontName.c_str());
	if (font == nullptr)
		return;

	m_text.setFont(*font);
	m_text.setPosition(data.offsetX, data.offsetY);
	m_text.setFillColor(sf::Color(data.r, data.g, data.b, data.a));
	m_text.setCharacterSize(data.fontSize);

	if (strlen(data.initialValue.c_str()) > 0)
		SetText(data.initialValue.c_str());
}

void UI::Text::Initialize()
{
}

void UI::Text::Destroy()
{
}

void UI::Text::Update()
{
}

void UI::Text::Draw(Renderer& r) const
{
	r.Draw(m_text);
}
