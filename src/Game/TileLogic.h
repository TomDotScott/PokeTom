#ifndef TILELOGIC_H
#define TILELOGIC_H
#include "TileParser.h"
#include "TileRenderer.h"

class TileLogic
{
public:
	TileLogic(const TileMapData& data);

	std::vector<TileRenderData> BuildRenderData() const;

private:
	struct TileInstance
	{
		sf::Vector2f m_Position;
		const TileSheet::TileDefinition* m_Definition;
		const TileSheet* m_ParentSheet;
		std::string m_LayerName;
	};

	std::vector<TileInstance> m_tiles;
};

#endif
