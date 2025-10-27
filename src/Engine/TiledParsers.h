#ifndef TILEDPARSERS_H
#define TILEDPARSERS_H
#include <filesystem>
#include <string>
#include <vector>
#include "Factory.h"
#include "../Libs/hoxml.h"
#include "../Libs/nlohmann/json.hpp"

class TMJ final : public Factory<TMJ>
{
	friend class Factory<TMJ>;

public:
	struct Layer
	{
		uint32_t m_ID;
		std::vector<uint32_t> m_Data;
		uint32_t m_Height;
		uint32_t m_Width;
		std::string m_Name;
		int m_ZIndex;
	};

	struct TileSet
	{
		uint32_t m_FirstGid;
		std::filesystem::path m_TsxSource;
	};

	const std::vector<Layer>& GetLayers() const;
	const std::vector<TileSet>& GetTileSets() const;
	uint32_t GetNumColumns() const;
	uint32_t GetNumRows() const;


private:
	std::filesystem::path m_filePath;
	uint32_t m_height;
	uint32_t m_width;
	std::vector<Layer> m_layers;

	// The GIDs are sorted from highest to lowest
	std::vector<TileSet> m_tileSets;

	bool Init() override;
	explicit TMJ(std::filesystem::path path);
	bool ParseLayers(const nlohmann::basic_json<>& layersArray);
	bool ParseTileSets(const nlohmann::basic_json<>& tileSetsArray);
};


class TSX final : public Factory<TSX>
{
	friend class Factory<TSX>;

public:
	struct TileSet
	{
		std::string m_Name;
		uint32_t m_TileWidth;
		uint32_t m_TileHeight;
		uint32_t m_TileCount;
		uint32_t m_NumColumns;
	};

	struct Image
	{
		std::filesystem::path m_Source;
		uint32_t m_Height;
		uint32_t m_Width;
	};

	struct Tile
	{
		struct Property
		{
			std::string m_Name;

			enum class eType : uint8_t
			{
				Bool = 1 << 0,
				Int = 1 << 2,
				Float = 1 << 3
			} m_Type;

			union uValue
			{
				bool m_bValue;
				int m_iValue;
				float m_fValue;
			} m_Value;
		};

		uint32_t m_ID;
		std::vector<Property> m_Properties;
	};

	const std::filesystem::path& GetPath() const;
	const TileSet& GetTileSetInfo() const;
	const Image& GetImageInfo() const;
	const std::vector<Tile>& GetTiles() const;

private:
	std::filesystem::path m_filePath;
	TileSet m_tileSet;
	Image m_image;
	std::vector<Tile> m_tiles;

	explicit TSX(std::filesystem::path path);
	bool Init() override;
	bool ParseTileSet(const std::filesystem::path& parentFolderPath, hoxml_context_t*& context, const char* xml, size_t xmlLength);
	bool ParseImage(const std::filesystem::path& parentFolderPath, hoxml_context_t*& context, const char* xml, size_t xmlLength);
	bool ParseProperties(Tile& tile, hoxml_context_t*& context, const char* xml, size_t xmlLength);
	bool ParseTile(hoxml_context_t*& context, const char* xml, size_t xmlLength);
};

#endif
