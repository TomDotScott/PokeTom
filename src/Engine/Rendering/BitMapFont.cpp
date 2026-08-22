#include "BitMapFont.h"

#include "../Asserts.h"
#include "../TextureManager.h"


BitMapFont::BitMapFont(const hash_type& inputTextureHash,
                       const sf::IntRect fontArea,
                       glyph_table_t glyphs,
                       const float glyphHeight,
                       const float lineHeight) :
	m_sprite(*TEXTUREMANAGER.GetTexture(inputTextureHash)),
	m_glyphs(std::move(glyphs)),
	m_lineHeight(lineHeight),
	m_referenceHeight(glyphHeight),
	m_fontSize(16.f)
{
	m_sprite.setTextureRect(fontArea);
}

glyph_table_t BitMapFont::GenerateGlyphTable(const std::string_view charOrder,
                                             const int numCols,
                                             const sf::Vector2i cellSize,
                                             const sf::Vector2i origin)
{
	glyph_table_t table;

	for (size_t i = 0; i < charOrder.size(); ++i)
	{
		const size_t col = i % numCols;
		const size_t row = i / numCols;

		const sf::IntRect bounds{
			{
				static_cast<int>(origin.x + col * cellSize.x),
				static_cast<int>(origin.y + row * cellSize.y)
			},
			cellSize
		};

		table[charOrder[i]] = {
			.m_Rect = bounds,
			.m_Advance = static_cast<float>(cellSize.x)
		};
	}

	return table;
}

sf::VertexArray BitMapFont::BuildText(const std::string_view text, const sf::Vector2f& position) const
{
	sf::VertexArray vertices(sf::PrimitiveType::Triangles);
	sf::Vector2f cursor = position;

	for (char c : text)
	{
		if (c == '\n')
		{
			cursor.x = position.x;
			cursor.y += m_lineHeight;
		}

		ASSERT(m_glyphs.contains(c));
		const auto& glyph = m_glyphs.at(c);

		sf::Vector2f size(glyph.m_Rect.size.x, glyph.m_Rect.size.y);
		sf::Vector2f uvTL(glyph.m_Rect.position.x, glyph.m_Rect.position.y);
		sf::Vector2f uvBR(glyph.m_Rect.position.x + glyph.m_Rect.size.x, glyph.m_Rect.position.y + glyph.m_Rect.size.y);

		sf::Vertex tl{
			.position = { cursor.x, cursor.y },
			.color = sf::Color::White,
			.texCoords = { uvTL.x, uvTL.y }
		};
		sf::Vertex tr{
			.position = { cursor.x + size.x, cursor.y },
			.color = sf::Color::White,
			.texCoords = { uvBR.x, uvTL.y }
		};
		sf::Vertex br{
			.position = { cursor.x + size.x, cursor.y + size.y },
			.color = sf::Color::White,
			.texCoords = { uvBR.x, uvBR.y }
		};
		sf::Vertex bl{
			.position = { cursor.x, cursor.y + size.y },
			.color = sf::Color::White,
			.texCoords = { uvTL.x, uvBR.y }
		};

		vertices.append(tl);
		vertices.append(tr);
		vertices.append(br);
		vertices.append(tl);
		vertices.append(br);
		vertices.append(bl);

		cursor.x += glyph.m_Advance;
	}

	return vertices;
}

void BitMapFont::Render(sf::RenderWindow& window,
                        const sf::Vector2f& position,
                        const sf::VertexArray& verts,
                        sf::RenderStates states) const
{
	states.texture = &m_sprite.getTexture();

	states.transform.translate(position);

	float scale = std::round(m_fontSize / m_referenceHeight);
	states.transform.scale({ scale, scale });

	window.draw(verts, states);
}

void BitMapFont::SetFontSize(const float size)
{
	m_fontSize = size;
}
