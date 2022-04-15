/* Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
#include "ui/widget.h"

#include <sfml/Graphics/Text.hpp>

#include <heart/codegen/codegen.h>
#include <heart/copy_move_semantics.h>

namespace UI
{
	typedef std::basic_string<sf::Uint8> string_utf8;

	class Text : public Widget
	{
	public:
		SERIALIZE_STRUCT()
		struct TextData
		{
			float offsetX, offsetY;
			int r, g, b, a;
			int fontSize;
			SerializedDataPath fontName;
			SerializedString<64> initialValue;
		};

	private:
		sf::Text m_text;

	public:
		Text() = default;
		Text(TextData data);
		~Text() = default;

		USE_DEFAULT_COPY_SEMANTICS(Text);
		USE_DEFAULT_MOVE_SEMANTICS(Text);

		void SetText(const char* text)
		{
			m_text.setString(text);
		}

		void SetText(const wchar_t* text)
		{
			m_text.setString(text);
		}

		string_utf8 GetTextUtf8() const
		{
			return m_text.getString().toUtf8();
		}

		// Inherited via Widget
		virtual void Initialize() override;

		virtual void Destroy() override;

		virtual void Update() override;

		virtual void Draw(Renderer& r) const override;
	};
}
