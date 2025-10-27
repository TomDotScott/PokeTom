#ifndef TILELOGIC_H
#define TILELOGIC_H
#include "TileParser.h"
#include "TileRenderer.h"

class TileLogic
{
public:
	TileLogic(const TileMapData& data);

	std::vector<TileRenderData> BuildRenderData() const;
	const std::vector<TileLayerData>& GetLayerData() const;

private:
	struct TileInstance
	{
		sf::Vector2f m_Position;
		const TileSheet::TileDefinition* m_Definition;
		const TileSheet* m_ParentSheet;
		std::string m_LayerName;
	};

	std::unordered_map<std::string, int> m_zIndexes;
	std::vector<TileLayerData> m_layers;
	std::vector<TileInstance> m_tiles;
};

#endif
