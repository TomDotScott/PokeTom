#ifndef BITMAPFONT_H
#define BITMAPFONT_H
#include <unordered_map>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include "../Hash.h"

struct Glyph
{
	sf::IntRect m_Rect;
	float m_Advance;
};

typedef std::unordered_map<char, Glyph> glyph_table_t;

class BitMapFont
{
public:
	explicit BitMapFont(const hash_type& inputTextureHash,
	                    sf::IntRect fontArea,
	                    glyph_table_t glyphs,
	                    float glyphHeight,
	                    float lineHeight);

	static glyph_table_t GenerateGlyphTable(std::string_view charOrder, int numCols, sf::Vector2i cellSize,
	                                        sf::Vector2i origin);

	sf::VertexArray BuildText(std::string_view text, const sf::Vector2f& position) const;

	void Render(sf::RenderWindow& window,
	            const sf::Vector2f& position,
	            const sf::VertexArray& verts,
	            sf::RenderStates states = {}) const;

	void SetFontSize(float size);

private:
	sf::Sprite m_sprite;
	glyph_table_t m_glyphs;
	float m_lineHeight;

	float m_referenceHeight;
	float m_fontSize;
};
#endif // BITMAPFONT_H
