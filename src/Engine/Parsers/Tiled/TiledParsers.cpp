#include "TiledParsers.h"
#include <fstream>
#include <iostream>
#include "zlib.h"
#include <base64.h>
#include "../XML/XmlDocument.h"


namespace
{
	std::string DecompressZlib(const std::string& compressed)
	{
		z_stream stream{ };
		stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressed.data()));
		stream.avail_in = compressed.size();

		if (inflateInit(&stream) != Z_OK)
			throw std::runtime_error("inflateInit failed");

		std::string out;
		char buffer[32768];
		int status;
		do
		{
			stream.next_out = reinterpret_cast<Bytef*>(buffer);
			stream.avail_out = sizeof(buffer);
			status = inflate(&stream, Z_NO_FLUSH);
			if (status != Z_OK && status != Z_STREAM_END)
			{
				inflateEnd(&stream);
				throw std::runtime_error("inflate failed");
			}
			out.append(buffer, sizeof(buffer) - stream.avail_out);
		}
		while (status != Z_STREAM_END);

		inflateEnd(&stream);
		return out;
	}

	std::vector<uint32_t> BytesToTiles(const std::string& bytes)
	{
		const size_t count = bytes.size() / 4;
		std::vector<uint32_t> tiles(count);

		// interpret every 4 bytes as an unsigned 32-bit int (little-endian)
		for (size_t i = 0; i < count; ++i)
		{
			const auto b = reinterpret_cast<const unsigned char*>(bytes.data() + i * 4);
			tiles[i] =
			static_cast<uint32_t>(b[0]) |
			static_cast<uint32_t>(b[1]) << 8 |
			static_cast<uint32_t>(b[2]) << 16 |
			static_cast<uint32_t>(b[3]) << 24;
		}

		return tiles;
	}
}

TMJ::TMJ(std::filesystem::path path) :
	m_filePath(std::move(path)),
	m_height(0x69),
	m_width(0x69)
{
}

const std::vector<TMJ::Layer>& TMJ::GetLayers() const
{
	return m_layers;
}

const std::vector<TMJ::Portal>& TMJ::GetPortals() const
{
	return m_portals;
}

const std::vector<TMJ::SpawnPoint>& TMJ::GetSpawnPoints() const
{
	return m_spawnPoints;
}

const std::vector<TMJ::TileSet>& TMJ::GetTileSets() const
{
	return m_tileSets;
}

uint32_t TMJ::GetNumColumns() const
{
	return m_width;
}

uint32_t TMJ::GetNumRows() const
{
	return m_height;
}

uint32_t TMJ::GetTileHeight() const
{
	return m_tileHeight;
}

uint32_t TMJ::GetTileWidth() const
{
	return m_tileWidth;
}

const std::filesystem::path& TMJ::GetLevelScriptFilepath() const
{
	return m_levelScriptPath;
}

bool TMJ::Init()
{
	if (!exists(m_filePath))
	{
		std::cout << "Error: No filepath " << m_filePath << " exists!\n";
		return false;
	}

	std::ifstream f(m_filePath);

	if (!f.is_open())
	{
		std::cout << "Error: Failed to open " << m_filePath << "\n";
		return false;
	}

	nlohmann::json json = nlohmann::json::parse(f);

	m_height = json["height"];
	m_width = json["width"];

	m_tileHeight = json["tileheight"];
	m_tileWidth = json["tilewidth"];

	const auto& layersArray = json["layers"];
	if (!ParseLayersArray(layersArray))
	{
		std::cout << "Error: Failed to parse layers array! Dumping...\n" << layersArray << "\n";
		return false;
	}

	const auto& levelPropertiesArray = json["properties"];
	if (!ParsePropertiesArray(levelPropertiesArray))
	{
		std::cout << "Error: Failed to parse properties array! Scripting may not work for level: " << m_filePath << "\n";
	}

	const auto& tileSetsArray = json["tilesets"];
	if (!ParseTileSets(tileSetsArray))
	{
		return false;
	}

	return true;
}

bool TMJ::ParseLayersArray(const nlohmann::basic_json<>& layersArray)
{
	if (layersArray.type() != nlohmann::detail::value_t::array)
	{
		std::cout << "Error: typeof layersArray=" << layersArray.type_name() << "\n";
		return false;
	}

	std::vector<Layer> layers;
	std::vector<Portal> portals;
	std::vector<SpawnPoint> spawnPoints;
	for (int i = 0; i < layersArray.size(); ++i)
	{
		const auto& elem = layersArray[i];

		const auto& objectType = elem["type"];
		if (!objectType.is_string())
		{
			return false;
		}

		const std::string objectTypeString = objectType;

		if (objectTypeString == "objectgroup")
		{
			if (!ParseObjectLayerType(elem, portals, spawnPoints))
			{
				std::cerr << "TMJ::ParseLayersArray: Failed to parse Object Layers!\n";
				return false;
			}
		}
		else if (objectTypeString == "tilelayer")
		{
			if (!ParseTileLayerType(elem, i, layers))
			{
				std::cerr << "TMJ::ParseLayersArray: Failed to parse Tile Layers!\n";
				return false;
			}
		}
	}

	m_layers = layers;
	m_portals = portals;
	m_spawnPoints = spawnPoints;
	return true;
}

bool TMJ::ParsePropertiesArray(const nlohmann::basic_json<>& propertiesArray)
{
	if (propertiesArray.type() != nlohmann::detail::value_t::array)
	{
		std::cout << "Error: typeof propertiesArray=" << propertiesArray.type_name() << "\n";
		return false;
	}

	std::filesystem::path levelScriptPath;

	for (const auto& elem : propertiesArray)
	{
		const auto& propertyName = elem["name"];
		if (!propertyName.is_string())
		{
			return false;
		}

		const std::string propertyNameString = propertyName;

		if (propertyNameString == "LevelScript")
		{
			levelScriptPath = std::string{ elem["value"] };
			if (!exists(levelScriptPath))
			{
				std::cerr << "TMJ::ParsePropertiesArray: File at location " << levelScriptPath << " does not exist!\n";

				return false;
			}
		}
	}

	m_levelScriptPath = levelScriptPath;
	return true;
}


bool TMJ::ParsePortals(const nlohmann::basic_json<>& doorsLayerObj, std::vector<Portal>& portals)
{
	const auto& height = doorsLayerObj["height"];
	if (!height.is_number())
	{
		return false;
	}

	const auto& name = doorsLayerObj["name"];
	if (!name.is_string())
	{
		return false;
	}

	const auto& properties = doorsLayerObj["properties"];
	if (!properties.is_array())
	{
		return false;
	}

	std::string orientation = "UNKNOWN";
	for (const auto& property : properties)
	{
		const auto& propertyName = property["name"];
		if (!propertyName.is_string())
		{
			return false;
		}

		const std::string propertyNameAsString = propertyName;
		if (propertyNameAsString == "Orientation")
		{
			const auto& propertyVal = property["value"];
			if (!propertyVal.is_string())
			{
				return false;
			}

			orientation = propertyVal;
		}
		else
		{
			// TODO: Might support more properties in the future
			continue;
		}
	}

	if (orientation == "UNKNOWN")
	{
		std::cerr << "TMJ::ParsePortals: Failed to parse Orientation!\n";
		return false;
	}

	const auto& width = doorsLayerObj["width"];
	if (!width.is_number())
	{
		return false;
	}

	const auto& x = doorsLayerObj["x"];
	if (!x.is_number())
	{
		return false;
	}

	const auto& y = doorsLayerObj["y"];
	if (!y.is_number())
	{
		return false;
	}

	portals.push_back({
		name,
		height,
		width,
		x,
		y,
		orientation
	});

	return true;
}

bool TMJ::ParseSpawnPoint(const nlohmann::basic_json<>& spawnPointObject, std::vector<SpawnPoint>& spawnPoints)
{
	// TODO: Maybe some extra enforcement here?
	const std::string spawnPointName = spawnPointObject["name"];

	if (spawnPointName.empty())
	{
		return false;
	}

	SpawnPoint spawnPoint;

	spawnPoint.m_Name = spawnPointName;

	const auto& x = spawnPointObject["x"];
	if (!x.is_number_integer())
	{
		std::cerr << "TMJ::ParseSpawnPoint: X is not an integer!\n";
		return false;
	}

	spawnPoint.m_X = x;

	const auto& y = spawnPointObject["y"];
	if (!y.is_number_integer())
	{
		std::cerr << "TMJ::ParseSpawnPoint: Y is not an integer!\n";
		return false;
	}

	spawnPoint.m_Y = y;

	const auto& properties = spawnPointObject["properties"];
	if (!properties.is_array())
	{
		return false;
	}

	// There should only be one property for spawnpoints!
	int parsedProperties = 0;
	for (const auto& property : properties)
	{
		if (parsedProperties >= 1)
		{
			std::cerr << "TMJ::ParseSpawnPoint: Parsed " << parsedProperties << "properties on spawnPoint " <<
			spawnPointName << ". Check the TMJ file!\n";
			return false;
		}

		if (const auto& propertyName = property["name"];
			!propertyName.is_string() ||
			std::string{ propertyName } != "Orientation")
		{
			std::cerr << "TMJ::ParseSpawnPoint: Failed to parse property " << propertyName << " on spawnPoint " <<
			spawnPointName << "\n";
			return false;
		}

		const auto& orientation = property["value"];
		if (!orientation.is_string())
		{
			std::cerr << "TMJ::ParseSpawnPoint: Orientation value is not a string on spawnPoint " << spawnPointName <<
			"\n";
			return false;
		}

		const std::string orientationAsString = orientation;
		if (orientationAsString != "Up" &&
			orientationAsString != "Down" &&
			orientationAsString != "Left" &&
			orientationAsString != "Right")
		{
			std::cerr << "TMJ::ParseSpawnPoint: Unknown orientation " << orientationAsString << "\n";
			return false;
		}

		spawnPoint.m_Orientation = orientationAsString;
		parsedProperties++;
	}

	spawnPoints.emplace_back(spawnPoint);
	return true;
}

bool TSX::TileSet::IsValid() const
{
	if (m_Name.empty())
	{
		return false;
	}

	if (m_TileWidth == ~0U)
	{
		return false;
	}

	if (m_TileHeight == ~0U)
	{
		return false;
	}

	if (m_TileCount == ~0U)
	{
		return false;
	}

	if (m_NumColumns == ~0U)
	{
		return false;
	}

	return true;
}

bool TSX::Image::IsValid() const
{
	if (m_Source.empty())
	{
		return false;
	}

	if (m_Width == ~0U)
	{
		return false;
	}

	if (m_Height == ~0U)
	{
		return false;
	}

	return true;
}

bool TMJ::ParseTileSets(const nlohmann::basic_json<>& tileSetsArray)
{
	if (tileSetsArray.type() != nlohmann::detail::value_t::array)
	{
		std::cout << "Error: typeof layersArray=" << tileSetsArray.type_name() << "\n";
		return false;
	}

	std::vector<TileSet> tileSets;
	for (const auto& elem : tileSetsArray)
	{
		const auto& gid = elem["firstgid"];
		if (!gid.is_number_integer())
		{
			return false;
		}

		const auto& tsxSource = elem["source"];
		if (!tsxSource.is_string())
		{
			return false;
		}

		const std::string tsxFilePath = tsxSource;

		if (!std::filesystem::exists(tsxFilePath))
		{
			std::cout << "Filepath " << tsxFilePath << " doesn't exist!\n";
			return false;
		}

		const TileSet tileSet = { gid, tsxFilePath };
		tileSets.emplace_back(tileSet);
	}

	std::ranges::sort(tileSets,
					  [](const TileSet& a, const TileSet& b){
						  return a.m_FirstGid > b.m_FirstGid;
					  }
	);
	m_tileSets = tileSets;
	return true;
}

bool TMJ::ParseTileLayerType(const nlohmann::basic_json<>& layerObj, const int zIndex, std::vector<Layer>& layers)
{
	const auto& name = layerObj["name"];
	if (!name.is_string())
	{
		return false;
	}

	const auto& id = layerObj["id"];
	if (!id.is_number_integer())
	{
		return false;
	}

	const auto& height = layerObj["height"];
	if (!height.is_number_integer())
	{
		return false;
	}

	bool isPlayerLayer = false;
	const auto& layerClass = layerObj.find("class");
	if (layerClass != layerObj.end())
	{
		if (*layerClass == static_cast<std::string>("PLAYER_LAYER"))
		{
			isPlayerLayer = true;
		}
		else
		{
			return false;
		}
	}

	const auto& width = layerObj["width"];
	if (!width.is_number_integer())
	{
		return false;
	}

	const auto& data = layerObj["data"];
	if (!data.is_string())
	{
		return false;
	}

	const std::string stringData = data;
	const auto decompressed = DecompressZlib(base64_decode(stringData));
	const auto tileData = BytesToTiles(decompressed);

	Layer layer = {
		id,
		tileData,
		height,
		width,
		name,
		zIndex,
		isPlayerLayer
	};

	layers.emplace_back(layer);
	return true;
}

bool TMJ::ParseObjectLayerType(const nlohmann::basic_json<>& objLayerObj, std::vector<Portal>& portals,
							   std::vector<SpawnPoint>& spawnPoints)
{
	const auto& objectsArray = objLayerObj["objects"];
	if (!objectsArray.is_array())
	{
		std::cerr << "TMJ::ParseObjectLayerType: \"Objects\" is NOT an array!\n";
		return false;
	}

	for (const auto& object : objectsArray)
	{
		const std::string objectType = object["type"];

		// Determine what type of object it is (NPC spawn, Portal, SpawnPoint, etc)
		if (objectType == "Portal")
		{
			if (!ParsePortals(object, portals))
			{
				std::cerr << "TMJ::ParseObjectLayerType: Failed to parse portals layer!\n";
				return false;
			}
		}
		else if (objectType == "SpawnPoint")
		{
			if (!ParseSpawnPoint(object, spawnPoints))
			{
				std::cerr << "TMJ::ParseObjectLayerType: Failed to parse SpawnPoints layer!\n";
				return false;
			}
		}
		else
		{
			std::cerr << "Unable to parse object with name: " << object["name"] << "\n";
		}
	}
	return true;
}

const std::filesystem::path& TSX::GetPath() const
{
	return m_filePath;
}

const TSX::TileSet& TSX::GetTileSetInfo() const
{
	return m_tileSet;
}

const TSX::Image& TSX::GetImageInfo() const
{
	return m_image;
}

const std::vector<TSX::Tile>& TSX::GetTiles() const
{
	return m_tiles;
}

TSX::TSX(std::filesystem::path path) :
	m_filePath(std::move(path)),
	m_tileSet(),
	m_image()
{
}

bool TSX::Init()
{
	XmlDocument doc;
	doc.Load(m_filePath);
	const bool res = LoadFromXML(*doc.Root().Child("tileset"));
	if (!res)
	{
		return false;
	}

	return true;
}

bool TSX::LoadFromXML(const XmlNode& node)
{
	const TileSet tileSet{
		.m_Name = node.Attr("name", std::string{ }),
		.m_TileWidth = node.Attr("tilewidth", ~0U),
		.m_TileHeight = node.Attr("tileheight", ~0U),
		.m_TileCount = node.Attr("tilecount", ~0U),
		.m_NumColumns = node.Attr("columns", ~0U)
	};

	if (!tileSet.IsValid())
	{
		return false;
	}

	const XmlNode* imageNode = node.Child("image");
	if (imageNode == nullptr)
	{
		return false;
	}

	Image image{
		.m_Source = imageNode->Attr("source", std::string{ }),
		.m_Height = imageNode->Attr("height", ~0U),
		.m_Width = imageNode->Attr("width", ~0U)
	};

	const auto cp = std::filesystem::current_path();
	const auto combinedPaths = (cp / "data" / image.m_Source).lexically_normal();
	if (!std::filesystem::exists(combinedPaths))
	{
		return false;
	}

	image.m_Source = combinedPaths;

	if (!image.IsValid())
	{
		return false;
	}

	const auto tiles = node.Children("tile");
	m_tiles.clear();
	m_tiles.reserve(tiles.size());
	for (const auto& tileNode : tiles)
	{
		Tile tile;

		tile.m_ID = tileNode->Attr("id", ~0U);
		if (tile.m_ID == ~0U)
		{
			return false;
		}

		const auto tileProperties = tileNode->Child("properties")->Children("property");

		tile.m_Properties.clear();
		tile.m_Properties.reserve(tileProperties.size());

		for (const auto& tileProperty : tileProperties)
		{
			const std::string propertyName = tileProperty->Attr("name", std::string{ });
			if (propertyName.empty())
			{
				return false;
			}

			const std::string propertyType = tileProperty->Attr("type", std::string{ });
			if (propertyType.empty())
			{
				return false;
			}

			const std::string propertyValue = tileProperty->Attr("value", std::string{ });
			if (propertyValue.empty())
			{
				return false;
			}

			Tile::Property prop;
			prop.m_Name = propertyName;

			if (propertyType == "float")
			{
				prop.m_Type = Tile::Property::eType::Float;
				prop.m_Value.m_fValue = tileProperty->Attr("value", 0.f);
			}
			else if (propertyType == "int")
			{
				prop.m_Type = Tile::Property::eType::Int;
				prop.m_Value.m_iValue = tileProperty->Attr("value", 0);
			}
			else if (propertyType == "bool")
			{
				prop.m_Type = Tile::Property::eType::Bool;
				prop.m_Value.m_bValue = tileProperty->Attr("value", false);
			}
			else
			{
				std::cerr << "Unknown property type " << propertyType << "\n";
				return false;
			}

			tile.m_Properties.emplace_back(prop);
		}

		m_tiles.emplace_back(tile);
	}

	m_tileSet = tileSet;
	m_image = image;
	return true;
}
