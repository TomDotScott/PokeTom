#ifndef TILESHEET_H
#define TILESHEET_H
#include <cstdint>
#include <filesystem>
#include <unordered_map>

#include "../Engine/Hash.h"
#include "../Engine/Parsers/Tiled/TiledParsers.h"

class TileSheet
{
public:
	struct TileDefinition
	{
		enum Properties : uint32_t
		{
			None = 0,

			IsBarrier = 1 << 0,
			IsBreakable = 1 << 1,
			IsGrass = 1 << 2,
			IsWater = 1 << 3,
			IsDoor = 1 << 4,

			All = 1 << 16
		};

		// ID from the TMJ file
		uint32_t m_GlobalID;
		// ID from the TSX file
		uint32_t m_LocalID;
		uint32_t m_Flags;
		hash_type m_ParentTileSetName;
	};

	explicit TileSheet(const std::shared_ptr<TSX>& tsxParser, uint32_t firstGID);
	const TileDefinition* GetTileDefinition(uint32_t localID) const;

	hash_type GetSpriteSheetResourceName() const;
	uint32_t GetNumColumns() const;
	uint32_t GetFirstGID() const;

	uint32_t GetTileWidth() const;
	uint32_t GetTileHeight() const;

private:
	hash_type m_name;
	uint32_t m_firstGID;

	uint32_t m_numColumns;

	uint32_t m_tileWidth;
	uint32_t m_tileHeight;

	hash_type m_spriteSheetResourceName;
	std::unordered_map<uint32_t, TileDefinition> m_tiles;

	bool LoadTexture(const std::filesystem::path& filepath);
	void BuildTileDefinitions(const std::shared_ptr<TSX>& tsxParser);
};

#endif
