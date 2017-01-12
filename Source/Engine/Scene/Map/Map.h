// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_MAP_MAP_
#define _ENGINE_MAP_MAP_

#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Vector2.h"

#include "Engine/Scene/Animation.h"

#include "Engine/Scene/Camera.h"
#include "Engine/Scene/Tickable.h"
#include "Engine/Renderer/Drawable.h"

#include "Engine/Scene/Collision/CollisionManager.h"

#include "Engine/Scene/Map/Blocks/MapFileHeaderBlock.h"

#define EDITOR_CAMERA_RAYCAST_LENGTH 32.0f

class EditorGrid;
class Atlas;
class AtlasHandle;
class Map;
struct MapTile;

#define DEFAULT_MAP_WIDTH		32
#define DEFAULT_MAP_HEIGHT		32
#define DEFAULT_MAP_DEPTH		5
#define MAP_LAYER_DEPTH			4000.0f

// [Warning] If you add stuff to this make sure to update script references, editor references
// and the g_map_tile_collision_colors array in Map.cpp. Arrgh, should refactor this! To many dependencies.
struct MapTileCollisionType
{
	enum Type
	{
		None		= 0,
		Solid		= 1,
		Player_Only = 2,

		Jump_Down	= 3, // |
		Jump_Up		= 4, // |
		Jump_Left	= 5, // |
		Jump_Right	= 6, // Deprecated. Legacy only.

		Enemy_Only	= 7,

		Path_Blocking = 8,

		COUNT
	};
};

struct AnimatedTile
{
//	MEMORY_ALLOCATOR(AnimatedTile, "Scene");

public:
	u16					X;
	u16					Y;
	MapTile*			Tile;

	AnimatedTile(int x, int y, MapTile* tile)
		: X(x)
		, Y(y)
		, Tile(tile)
	{
	}
};

#pragma pack(push, 1) // We have to create a lot of large arrays of MapTile, 
					  // so better to be tightly packed than fast to access.

// TODO: Better idea is to palletise this and just use a u16 index per tile.

struct MapTile
{
//	MEMORY_ALLOCATOR(MapTile, "Scene");

public:
	u16							Frame;						// 6
	u16							CollisionFrame;				// 8

	Color						TileColor;					// 4
	Animation*					TileAnimation;				// 13

	bool						FlipHorizontal : 1;
	bool						FlipVertical : 1;
	MapTileCollisionType::Type	Collision : 6;				// 9

};
#pragma pack(pop)

class MapLayer : public IDrawable
{
	MEMORY_ALLOCATOR(MapLayer, "Scene");

private:
	Map* m_map;
	int m_width;
	int m_height;
	int m_layer_index;

	enum
	{
		MAX_MAP_SEGMENT_SIZE_X = 64,
		MAX_MAP_SEGMENT_SIZE_Y = 64,
	};

	Geometry* m_map_opaque_geometry;
	Geometry* m_map_transparent_geometry;
	Geometry* m_collision_geometry;

	MapTile* m_tiles;

	bool m_visible;
	bool m_collision_visible;

	bool m_dirty;

	std::vector<AnimatedTile> m_animated_tiles;
	std::vector<Animation*> m_tile_animations;

public:

	~MapLayer();
	MapLayer(Map* map, int layer_index, int width, int height);
	
	void Add_Draw_Instances(const FrameTime& time, std::vector<DrawInstance>& instances) ;

	// Drawing/Updating
	void Tick(const FrameTime& time);
	void Draw(const FrameTime& time, RenderPipeline* pipeline, const DrawInstance& instance);
	void Draw_Collision(const FrameTime& time, RenderPipeline* pipeline);

	// Tile stuff
	MapTile* Get_Tile(int x, int y);
	MapTile* Get_Tiles();
	void Clear();
	
	// Collision shenanigans!
	void Create_Collision();

	// Regeneration of render data.
	void Mark_Dirty();
	bool Is_Dirty();
	void Regenerate();

	bool Get_Visible();
	void Set_Visible(bool value);
	
	bool Get_Collision_Visible();
	void Set_Collision_Visible(bool value);

	float Get_Draw_Depth();
	bool  Is_Visible_From(Rect2D viewport);

};

class Map : public Tickable
{
	MEMORY_ALLOCATOR(Map, "Scene");

private:
	int m_width;
	int m_height;
	int m_depth;

	MapLayer** m_layers;

	AtlasHandle* m_tileset;
	int m_tile_width;
	int m_tile_height;

	int m_map_ids;

	bool m_collision_dirty;

	std::vector<Animation> m_all_animations;

	MapFileHeaderBlock m_header;
	Pixelmap* m_preview_pixmap;
	Texture* m_preview_texture;

	bool m_owns_preview_pixmap;

	Vector4 m_color_tint;

	float m_dirty_time;
	float m_regenerate_defer_interval;

protected:
	friend class MapLayer;

	bool m_draw_active_layer_tint;
	int m_draw_active_layer_tint_index;

private:

	void Free_Layers(MapLayer** layers);

public:	

	Map();
	~Map();

	// Base functions.
	void			Tick(const FrameTime& time);
	
	// Sizing settings.
	void			Resize(int width, int height, int depth, bool persist = false);
	int				Get_Width();
	int				Get_Height();
	int				Get_Depth();
	void			Mark_Dirty();

	// Collision shenanigans!
	void			Create_Collision();
	void			Mark_Collision_Dirty();
	void			Set_Regenerate_Defer_Time(float time);

	// Tile modification
	MapTile*				Get_Tile(int x, int y, int z);
	MapLayer*				Get_Layer(int z);
	void					Set_Tileset(AtlasHandle* atlas);
	AtlasHandle*			Get_Tileset();
	int						Get_Tile_Width();
	int						Get_Tile_Height();
	Vector2					Get_Tile_Size();
	std::vector<Animation>	Get_All_Tile_Animations();

	Vector4 Get_Color_Tint();
	void Set_Color_Tint(Vector4 tint);

	void Set_Active_Layer_Tint(bool bTinted, int activeLayerIndex);

	// Set header information.
	MapFileHeaderBlock& Get_Map_Header();
	void Set_Map_Header(MapFileHeaderBlock block);
	void Apply_Header_Changes();

	// Preview Image.
	void Set_Preview_Pixmap(Pixelmap* pix, bool owns_pixmap = false);
	Pixelmap* Get_Preview_Pixmap();
	Texture* Get_Preview_Image();

	s32 Create_Map_ID()
	{
		return m_map_ids++;
	}

	void Reset_Map_IDs()
	{
		m_map_ids = 1;
	}

};

#endif

