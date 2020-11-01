#ifndef __MAP_H__
#define __MAP_H__

#include "Module.h"
#include "List.h"
#include "Point.h"

#include "PugiXml\src\pugixml.hpp"

struct TileSet
{
	SString	name;
	int	firstgid;
	int margin;
	int	spacing;
	int	tileWidth;
	int	tileHeight;

	SDL_Texture* texture;
	int	texWidth;
	int	texHeight; 
	int	numTilesWidth;
	int	numTilesHeight;
	int	offsetX;
	int	offsetY;
	int tilecount;

	SDL_Rect GetTileRect(int id) const;
};

enum MapTypes
{
	MAPTYPE_UNKNOWN = 0,
	MAPTYPE_ORTHOGONAL,
	MAPTYPE_ISOMETRIC,
	MAPTYPE_STAGGERED
};


struct Properties
{
	struct Property
	{
		SString	name;
		int value;
	};
	
	~Properties()
	{
		ListItem<Property*>* listD;
		listD = list.start;
		while (listD != nullptr)
		{
			RELEASE(listD->data);
			listD = listD->next;
		}
		list.clear();
	}

	// Method to ask for the value of a custom property
	int GetProperty(const char* name, int default_value = 0) const;

	List<Property*> list;
};

struct MapLayer
{
	SString	name = "";
	int width = 0;
	int height = 0;
	uint* data;

	Properties properties;

	MapLayer() : data(NULL)
	{}

	~MapLayer()
	{
		RELEASE(data);
	}
	
	// Get the value of x,y
	inline uint Get(int x, int y) const
	{
		return data[(y * width) + x];
	}
};

struct MapData
{
	int width;
	int	height;
	int	tileWidth;
	int	tileHeight;
	SDL_Color backgroundColor;
	MapTypes type;
	List<TileSet*> tilesets;

	List<MapLayer*> layers;
};

class Map : public Module
{
public:

    Map();

    // Destructor
    virtual ~Map();

    // Called before render is available
    bool Awake(pugi::xml_node& conf);

    // Called each loop iteration
    void Draw();

    // Called before quitting
    bool CleanUp();

    // Load new map
    bool Load(const char* path);

	// Translates x,y coordinates from map positions to world positions
	iPoint MapToWorld(int x, int y) const;

	// Translates x,y coordinates from  world positions to map positions 
	iPoint WorldToMap(int x, int y) const;

private:

	// L03: Methods to load all required map data
	bool LoadMap();
	bool LoadTilesetDetails(pugi::xml_node& tileset_node, TileSet* set);
	bool LoadTilesetImage(pugi::xml_node& tileset_node, TileSet* set);
	bool LoadLayer(pugi::xml_node& node, MapLayer* layer);

	// Load a group of properties 
	bool LoadProperties(pugi::xml_node& node, Properties& properties);

	// Pick the right Tileset based on a tile id
	TileSet* GetTilesetFromTileId(int id) const;

public:

	MapData data;

private:

    pugi::xml_document mapFile;
    SString folder;
    bool mapLoaded;

	bool* drawColl;
};

#endif // __MAP_H__