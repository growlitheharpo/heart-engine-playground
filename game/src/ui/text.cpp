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

	text_.setFont(*font);
	text_.setPosition(data.offsetX, data.offsetY);
	text_.setFillColor(sf::Color(data.r, data.g, data.b, data.a));
	text_.setCharacterSize(data.fontSize);

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
	r.Draw(text_);
}
