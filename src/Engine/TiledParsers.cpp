#include "TiledParsers.h"
#include <fstream>
#include <iostream>
#include "zlib.h"
#include "../Engine/Globals.h"
#include <base64.h>

namespace
{
	std::string DecompressZlib(const std::string& compressed)
	{
		z_stream stream{};
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
		} while (status != Z_STREAM_END);

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

	const auto& layersArray = json["layers"];
	if (!ParseLayersArray(layersArray))
	{
		std::cout << "Error: Failed to parse layers array! Dumping...\n" << layersArray << "\n";
		return false;
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

	std::sort(tileSets.begin(),
		tileSets.end(),
		[](const TileSet& a, const TileSet& b)
		{
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
		zIndex
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
	std::string line, text;
	std::ifstream in(m_filePath);
	while (std::getline(in, line))
	{
		text += line + "\n";
	}

	if (text.empty())
	{
		return false;
	}

	const char* content = text.c_str();

	const size_t content_length = strlen(content);

	hoxml_context_t* hoxml_context = new hoxml_context_t();
	const auto buffer = static_cast<char*>(malloc(content_length * 2));

	hoxml_init(hoxml_context, buffer, content_length * 2);

	// Loop until the "end of document" code is returned
	hoxml_code_t code = hoxml_parse(hoxml_context, content, content_length);
	while (code != HOXML_END_OF_DOCUMENT)
	{
		if (code == HOXML_ELEMENT_BEGIN)
		{
			if (strcmp("tileset", hoxml_context->tag) == 0)
			{
				if (!ParseTileSet(m_filePath.parent_path(), hoxml_context, content, content_length))
				{
					return false;
				}
			}
		}
		if (code == HOXML_ERROR_TAG_MISMATCH)
		{
			printf("TSX::Init: Start tag did not match end tag on line %u\n", hoxml_context->line);
			return false;
		}

		code = hoxml_parse(hoxml_context, content, content_length);
	}

	delete hoxml_context;
	free(buffer);
	return true;
}

bool TSX::ParseTileSet(const std::filesystem::path& parentFolderPath, hoxml_context_t*& context, const char* xml,
	size_t xmlLength)
{
	TileSet tileSet{};
	hoxml_code_t code = HOXML_ELEMENT_BEGIN;
	do
	{
		if (code == HOXML_ELEMENT_BEGIN)
		{
			if (strcmp("image", context->tag) == 0)
			{
				if (!ParseImage(parentFolderPath, context, xml, xmlLength))
				{
					std::cout << "TSX::ParseTileSet: Failed to load Image tag!\n";
					return false;
				}
			}
			else if (strcmp("tile", context->tag) == 0)
			{
				if (!ParseTile(context, xml, xmlLength))
				{
					std::cout << "TSX::ParseTileSet: Failed to load Tile tag!\n";
					return false;
				}
			}
		}
		else if (code == HOXML_ELEMENT_END)
		{
			if (strcmp("tileset", context->tag) == 0)
			{
				m_tileSet = tileSet;
				return true;
			}
		}
		else if (code == HOXML_ATTRIBUTE)
		{
			if (strcmp("name", context->attribute) == 0)
			{
				tileSet.m_Name = context->value;
			}
			else if (strcmp("tilewidth", context->attribute) == 0)
			{
				tileSet.m_TileWidth = std::stoi(context->value);
			}
			else if (strcmp("tileheight", context->attribute) == 0)
			{
				tileSet.m_TileHeight = std::stoi(context->value);
			}
			else if (strcmp("tilecount", context->attribute) == 0)
			{
				tileSet.m_TileCount = std::stoi(context->value);
				m_tiles.reserve(tileSet.m_TileCount);
			}
			else if (strcmp("columns", context->attribute) == 0)
			{
				tileSet.m_NumColumns = std::stoi(context->value);
			}
		}

		code = hoxml_parse(context, xml, xmlLength);
	} while (code != HOXML_END_OF_DOCUMENT);

	std::cout << "TSX::ParseTileSet: Reached the end of the file without closing the tileset tag!\n";
	return false;
}

bool TSX::ParseImage(const std::filesystem::path& parentFolderPath, hoxml_context_t*& context, const char* xml,
	size_t xmlLength)
{
	Image image{};
	hoxml_code_t code = HOXML_ELEMENT_BEGIN;
	do
	{
		if (code == HOXML_ELEMENT_END)
		{
			if (strcmp("image", context->tag) == 0)
			{
				m_image = image;
				return true;
			}
		}
		else if (code == HOXML_ATTRIBUTE)
		{
			if (strcmp("source", context->attribute) == 0)
			{
				const auto combinedPaths = (std::filesystem::current_path() / "data" / context->value).lexically_normal();

				if (std::filesystem::exists(combinedPaths))
				{
					image.m_Source = combinedPaths;
				}
				else
				{
					return false;
				}
			}
			else if (strcmp("width", context->attribute) == 0)
			{
				image.m_Width = std::stoi(context->value);
			}
			else if (strcmp("height", context->attribute) == 0)
			{
				image.m_Height = std::stoi(context->value);
			}
		}

		code = hoxml_parse(context, xml, xmlLength);
	} while (code != HOXML_END_OF_DOCUMENT);

	std::cout << "TSX::ParseImage: Reached the end of the file without closing the image tag!\n";
	return false;
}

bool TSX::ParseProperties(Tile& tile, hoxml_context_t*& context, const char* xml, size_t xmlLength)
{
	std::vector<Tile::Property> properties;
	hoxml_code_t code = HOXML_ELEMENT_BEGIN;
	do
	{
		if (code == HOXML_ELEMENT_BEGIN)
		{
			if (strcmp("properties", context->tag) == 0)
			{
				Tile::Property currentProperty{};
				while (code != HOXML_END_OF_DOCUMENT)
				{
					if (code == HOXML_ATTRIBUTE)
					{
						// Just in case the "properties" element has some attributes
						if (strcmp("properties", context->tag) == 0)
						{
							continue;
						}

						if (strcmp("name", context->attribute) == 0)
						{
							currentProperty.m_Name = context->value;
						}
						else if (strcmp("type", context->attribute) == 0)
						{
							if (strcmp("bool", context->value) == 0)
							{
								currentProperty.m_Type = Tile::Property::eType::Bool;
							}
							else if (strcmp("int", context->value) == 0)
							{
								currentProperty.m_Type = Tile::Property::eType::Int;
							}
							else if (strcmp("float", context->value) == 0)
							{
								currentProperty.m_Type = Tile::Property::eType::Float;
							}
						}
						else if (strcmp("value", context->attribute) == 0)
						{
							// TODO: Assert that the type has been set properly first!
							switch (currentProperty.m_Type)
							{
							case Tile::Property::eType::Bool:
								currentProperty.m_Value.m_bValue = strcmp("true", context->value) == 0;
								break;
							case Tile::Property::eType::Int:
								currentProperty.m_Value.m_iValue = std::stoi(context->value);
								break;
							case Tile::Property::eType::Float:
								currentProperty.m_Value.m_fValue = std::stof(context->value);
								break;
							default:
								std::cout << "TSX::ParseProperties: UNHANDLED TYPE\n";
								return false;
							}
						}
					}
					else if (code == HOXML_ELEMENT_END)
					{
						if (strcmp("properties", context->tag) == 0)
						{
							tile.m_Properties = properties;
							return true;
						}
						if (strcmp("property", context->tag) == 0)
						{
							properties.push_back(currentProperty);
							currentProperty = {};
						}
					}

					code = hoxml_parse(context, xml, xmlLength);
				}
			}
		}

		code = hoxml_parse(context, xml, xmlLength);
	} while (code != HOXML_END_OF_DOCUMENT);

	std::cout << "TSX::ParseProperties: Reached the end of the file without closing the image tag!\n";
	return false;
}

bool TSX::ParseTile(hoxml_context_t*& context, const char* xml, size_t xmlLength)
{
	Tile tile{};
	hoxml_code_t code = HOXML_ELEMENT_BEGIN;
	do
	{
		if (code == HOXML_ELEMENT_BEGIN)
		{
			if (strcmp("properties", context->tag) == 0)
			{
				if (!ParseProperties(tile, context, xml, xmlLength))
				{
					std::cout << "TSX::ParseTile: Failed to load properties list!\n";
					return false;
				}
			}
		}
		else if (code == HOXML_ELEMENT_END)
		{
			if (strcmp("tile", context->tag) == 0)
			{
				m_tiles.emplace_back(tile);
				return true;
			}
		}
		else if (code == HOXML_ATTRIBUTE)
		{
			if (strcmp("id", context->attribute) == 0)
			{
				tile.m_ID = std::stoi(context->value);
			}
		}

		code = hoxml_parse(context, xml, xmlLength);
	} while (code != HOXML_END_OF_DOCUMENT);

	std::cout << "TSX::ParseImage: Reached the end of the file without closing the image tag!\n";
	return false;
}
