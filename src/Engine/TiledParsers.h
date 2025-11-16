#ifndef TILEDPARSERS_H
#define TILEDPARSERS_H
#include <filesystem>
#include <string>
#include <vector>
#include "Factory.h"
#include <json.hpp>
#include <hoxml.h>

class TMJ final : public Factory<TMJ>
{
	friend class Factory<TMJ>;

public:
	struct Portal
	{
		// TODO: This should probably be a hash of the asset
		// There really needs to be some proper asset management in this
		// little engine, but that sounds v complicated to do at this stage
		std::filesystem::path m_LevelToLoad;
		uint32_t m_Height;
		uint32_t m_Width;

		// Needs to be divided by the tile size
		uint32_t m_X;

		// Needs to be divided by the tile size
		uint32_t m_Y;

		// Comma-delimited "Up", "Down", "Left", or "Right"
		std::string m_Orientation;

		uint32_t m_SpawnPointID;
	};

	struct SpawnPoint
	{
		// The ID comes from the "name" of the Object in the Object Layer
		// There is an ID field, but it increments by 1 every time an
		// object is created, so may not be reliable to use when
		// setting the entrance and exits of the doors
		uint32_t m_ID;

		// Needs to be divided by the tile size
		uint32_t m_X;

		// Needs to be divided by the tile size
		uint32_t m_Y;

		// Either "Up", "Down", "Left", or "Right"
		// Converted into an Enum when the TileLogic creates
		// the TileMapData instance
		std::string m_Orientation;
	};

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
	const std::vector<Portal>& GetPortals() const;
	const std::vector<SpawnPoint>& GetSpawnPoints() const;
	const std::vector<TileSet>& GetTileSets() const;
	uint32_t GetNumColumns() const;
	uint32_t GetNumRows() const;


private:
	std::filesystem::path m_filePath;
	uint32_t m_height;
	uint32_t m_width;
	std::vector<Layer> m_layers;
	std::vector<Portal> m_portals;
	std::vector<SpawnPoint> m_spawnPoints;

	// The GIDs are sorted from highest to lowest
	std::vector<TileSet> m_tileSets;

	bool Init() override;
	explicit TMJ(std::filesystem::path path);
	bool ParseLayersArray(const nlohmann::basic_json<>& layersArray);
	bool ParseTileSets(const nlohmann::basic_json<>& tileSetsArray);
	static bool ParseTileLayerType(const nlohmann::basic_json<>& layerObj, int zIndex, std::vector<Layer>& layers);
	static bool ParseObjectLayerType(const nlohmann::basic_json<>& objLayerObj, std::vector<Portal>& portals, std::vector<SpawnPoint>& spawnPoints);
	static bool ParsePortals(const nlohmann::basic_json<>& doorsLayerObj, std::vector<Portal>& portals);
	static bool ParseSpawnPoint(const nlohmann::basic_json<>& spawnPointObject, std::vector<SpawnPoint>& spawnPoints);
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
