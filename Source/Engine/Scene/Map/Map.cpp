// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scene/Map/Map.h"
#include "Engine/Scene/Map/MapFile.h"

#include "Engine/Renderer/RenderPipeline.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Textures/Pixelmap.h"
#include "Engine/Renderer/PrimitiveRenderer.h"

#include "Engine/Scene/Pathing/PathManager.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

#include "Engine/Platform/Platform.h"

#include "Generic/Types/Color.h"

#include <vector>

extern Color g_map_tile_collision_colors[] = 
{
	Color(255, 255, 255, 128),
	Color(0, 0, 255, 128),
	Color(255, 0, 0, 128),

	Color(255, 0, 255, 128),
	Color(255, 0, 255, 128),
	Color(255, 0, 255, 128),
	Color(255, 0, 255, 128),

	Color(0, 255, 0, 128),

	Color(255, 255, 0, 128)
};

MapLayer::MapLayer(Map* map, int layer_depth, int width, int height)
	: m_map(map)
	, m_width(width)
	, m_height(height)
	, m_map_opaque_geometry(NULL)
	, m_map_transparent_geometry(NULL)
	, m_collision_geometry(NULL)
	, m_layer_index(layer_depth)
	, m_visible(true)
	, m_collision_visible(false)
{
	MemoryAllocator* allocator = MemoryManager::Get_Allocator<MapTile>("Scene");
	m_tiles = (MapTile*)allocator->Alloc(sizeof(MapTile) * width * height);

	Clear();
	Mark_Dirty();

	Actor::Set_Global_Layer_Depth_Multiplier(MAP_LAYER_DEPTH);

	Set_Render_Slot("geometry");
}

MapLayer::~MapLayer()
{
	SAFE_DELETE(m_map_transparent_geometry);
	SAFE_DELETE(m_map_opaque_geometry);
	SAFE_DELETE(m_collision_geometry);

	MemoryAllocator* allocator = MemoryManager::Get_Allocator<MapTile>("Scene");
	allocator->Free(m_tiles);
	m_tiles = NULL;
}

bool MapLayer::Get_Visible()
{
	return m_visible;
}

void MapLayer::Set_Visible(bool value)
{
	m_visible = value;
}

bool MapLayer::Get_Collision_Visible()
{
	return m_collision_visible;
}

void MapLayer::Set_Collision_Visible(bool value)
{
	m_collision_visible = value;
}

void MapLayer::Clear()
{
	for (int x = 0; x < m_width; x++)
	{
		for (int y = 0; y < m_height; y++)
		{
			MapTile* tile = Get_Tile(x, y);
			tile->TileColor				= Color::White;
			tile->FlipHorizontal	= false;
			tile->FlipVertical		= false;
			tile->Frame				= 0xFFFF;
			tile->Collision			= MapTileCollisionType::None;
			tile->CollisionFrame	= 0xFFFF;
			tile->TileAnimation			= NULL;
		}
	}

	for (std::vector<Animation*>::iterator iter = m_tile_animations.begin(); iter != m_tile_animations.end(); iter++)
	{
		Animation* anim_tile = *iter;
		SAFE_DELETE(anim_tile);
	}

	m_animated_tiles.clear();
	m_tile_animations.clear();
}

MapTile* MapLayer::Get_Tile(int x, int y)
{
	return &m_tiles[(y * m_width) + x];
}

MapTile* MapLayer::Get_Tiles()
{
	return m_tiles;
}

void MapLayer::Mark_Dirty()
{
	m_dirty = true;
}

bool MapLayer::Is_Dirty()
{
	return m_dirty;
}

float MapLayer::Get_Draw_Depth()
{
	return (m_layer_index * MAP_LAYER_DEPTH);
}

bool MapLayer::Is_Visible_From(Rect2D viewport)
{
	return m_visible; // Map layers are always visible and always drawn.
}

void MapLayer::Regenerate()
{
	Renderer* renderer = Renderer::Get();

	// Calculate some basic bits and pieces.
	Atlas* tileset = m_map->Get_Tileset()->Get();

	// Can't regenerate if we have no tileset! Defer
	// until we do have one.
	if (tileset == NULL && tileset->Get_Frame_Count() <= 0)
	{
		return;
	}
	
	// Delete old meshes.
	SAFE_DELETE(m_map_opaque_geometry);
	SAFE_DELETE(m_map_transparent_geometry);
	SAFE_DELETE(m_collision_geometry);
	
	DBG_LOG("Regenerated map layer %i mesh.", m_layer_index);

	int		cells					= m_width * m_height;
	float	layer_depth				= (m_layer_index * MAP_LAYER_DEPTH);
	float	collision_layer_depth	= ((m_map->Get_Depth() + 1) * MAP_LAYER_DEPTH);
	float	cell_width				= (float)m_map->Get_Tile_Width();
	float	cell_height				= (float)m_map->Get_Tile_Height();
	int		opaque_tile_count		= 0;
	int		transparent_tile_count	= 0;
	int		collision_count			= 0;

	AtlasFrame* default_frame = tileset->Get_Frame_By_Index(0);

	m_animated_tiles.clear();

	// Count valid tiles.
	for (int x = 0; x < m_width; x++)
	{
		for (int y = 0; y < m_height; y++)
		{
			MapTile* tile = Get_Tile(x, y);
			tile->TileAnimation = NULL;			

			AtlasAnimation* anim = tileset->Get_Animation_By_Start_Frame(tile->Frame);
			if (anim != NULL)
			{
				for (std::vector<Animation*>::iterator iter = m_tile_animations.begin(); iter != m_tile_animations.end(); iter++)
				{
					Animation* anim_tile = *iter;
					if (anim_tile->Get_Atlas_Animation() == anim)
					{
						tile->TileAnimation = anim_tile;
						break;
					}
				}

				if (tile->TileAnimation == NULL)
				{
					m_tile_animations.push_back(new Animation(anim));
					tile->TileAnimation = m_tile_animations.at(m_tile_animations.size() - 1);
				}

				m_animated_tiles.push_back(AnimatedTile(x, y, tile));
			}
			
			if (!(tile->Frame == 0xFFFF || tile->TileAnimation != NULL))
			{
				AtlasFrame* frame = tileset->Get_Frame_By_Index(tile->Frame);
				if (tile->TileColor.A < 255 || (frame != NULL && frame->HasSemiTransparentPixels))
				{
					transparent_tile_count++;
				}
				else
				{
					opaque_tile_count++;
				}
			}

			if (!(tile->CollisionFrame == 0xFFFF || tile->Collision == MapTileCollisionType::None))
			{
				collision_count++;
			}
		}
	}

	// Create geometry for map and collision.
	GeometryBufferFormat format;
	format.Add(GeometryBufferElementType::Position, GeometryBufferDataType::Float3);
	format.Add(GeometryBufferElementType::TexCoord, GeometryBufferDataType::Float2);
	format.Add(GeometryBufferElementType::Color,    GeometryBufferDataType::Float4);

	m_map_opaque_geometry = renderer->Create_Geometry(format, PrimitiveType::Quad, opaque_tile_count, false);	
	m_map_transparent_geometry = renderer->Create_Geometry(format, PrimitiveType::Quad, transparent_tile_count, false);	
	m_collision_geometry = renderer->Create_Geometry(format, PrimitiveType::Quad, collision_count, false);	

	DBG_LOG("Map Layer %i Stats:", m_layer_index);
	DBG_LOG("\tOpaque Tiles: %i", opaque_tile_count);
	DBG_LOG("\tTransparent Tiles: %i", transparent_tile_count);
	DBG_LOG("\tCollision Tiles: %i", collision_count);

	float* map_opaque_buffer = (float*)m_map_opaque_geometry->Lock_Vertices();
	float* map_transparent_buffer = (float*)m_map_transparent_geometry->Lock_Vertices();
	float* collision_buffer = (float*)m_collision_geometry->Lock_Vertices();

	if (map_opaque_buffer != NULL && map_transparent_buffer != NULL)
	{
		// Fill map buffer.
		for (int x = 0; x < m_width; x++)
		{
			for (int y = 0; y < m_height; y++)
			{
				MapTile* tile = Get_Tile(x, y);

				if (tile->Frame == 0xFFFF || tile->TileAnimation != NULL)
				{
					continue;
				}

				AtlasFrame* frame = tileset->Get_Frame_By_Index(tile->Frame);
				bool is_transparent = tile->TileColor.A < 255 || (frame != NULL && frame->HasSemiTransparentPixels);
				
				if (frame == NULL)
				{
					//DBG_LOG("Map containes invalid frame index %i, using default frame. Possible memory corruption or old map file?", tile->Frame);
					frame = default_frame;
				}

				float offset_x = x * cell_width;
				float offset_y = y * cell_height;

				Vector3 normal(0.0f, 0.0f, 0.0f);

				float left		= offset_x;
				float top		= offset_y;
				float right		= left + cell_width;
				float bottom	= top + cell_height;

				float uv_left	= frame->UV.X;
				float uv_right	= frame->UV.X + frame->UV.Width;
				float uv_top	= 1.0f - frame->UV.Y;
				float uv_bottom = uv_top - frame->UV.Height;

				float r = tile->TileColor.R / 255.0f;
				float g = tile->TileColor.G / 255.0f;
				float b = tile->TileColor.B / 255.0f;
				float a = tile->TileColor.A / 255.0f;

				if (tile->FlipHorizontal == true)
				{
					float tmp = uv_left;
					uv_left = uv_right;
					uv_right = tmp;
				}

				if (tile->FlipVertical == true)
				{
					float tmp = uv_top;
					uv_top = uv_bottom;
					uv_bottom = tmp;
				}

				float* map_buffer = is_transparent ? map_transparent_buffer : map_opaque_buffer;

				*(map_buffer++) = left;				*(map_buffer++) = top;				*(map_buffer++) = layer_depth;
				*(map_buffer++) = uv_left;			*(map_buffer++) = uv_top;
				*(map_buffer++) = r;				*(map_buffer++) = g;				*(map_buffer++) = b;					*(map_buffer++) = a;

				*(map_buffer++) = left;				*(map_buffer++) = bottom;			*(map_buffer++) = layer_depth;
				*(map_buffer++) = uv_left;			*(map_buffer++) = uv_bottom;
				*(map_buffer++) = r;				*(map_buffer++) = g;				*(map_buffer++) = b;					*(map_buffer++) = a;

				*(map_buffer++) = right;			*(map_buffer++) = bottom;			*(map_buffer++) = layer_depth;
				*(map_buffer++) = uv_right;			*(map_buffer++) = uv_bottom;
				*(map_buffer++) = r;				*(map_buffer++) = g;				*(map_buffer++) = b;					*(map_buffer++) = a;

				*(map_buffer++) = right;			*(map_buffer++) = top;				*(map_buffer++) = layer_depth;
				*(map_buffer++) = uv_right;			*(map_buffer++) = uv_top;
				*(map_buffer++) = r;				*(map_buffer++) = g;				*(map_buffer++) = b;					*(map_buffer++) = a;

				if (is_transparent)
				{
					map_transparent_buffer = map_buffer;
				}
				else
				{
					map_opaque_buffer = map_buffer;
				}
			}
		}
	}
	
	if (collision_buffer != NULL)
	{
		// Fill collision buffer.
		for (int x = 0; x < m_width; x++)
		{
			for (int y = 0; y < m_height; y++)
			{
				MapTile* tile = Get_Tile(x, y);

				if (tile->CollisionFrame == 0xFFFF || tile->Collision == MapTileCollisionType::None)
				{
					continue;
				}
			
				AtlasFrame* frame = tileset->Get_Frame_By_Index(tile->CollisionFrame);
				if (frame == NULL)
				{
					DBG_LOG("Map containes invalid frame index %i, using default frame. Possible memory corruption or old map file?", tile->Frame);
					frame = default_frame;
				}

				float offset_x = x * cell_width;
				float offset_y = y * cell_height;

				Vector3 normal(0.0f, 0.0f, 0.0f);

				float left		= offset_x;
				float top		= offset_y;
				float right		= left + cell_width;
				float bottom	= top + cell_height;

				float uv_left	= frame->UV.X;
				float uv_right	= frame->UV.X + frame->UV.Width;
				float uv_top	= 1.0f - frame->UV.Y;
				float uv_bottom = uv_top - frame->UV.Height;

				Color collision_color = g_map_tile_collision_colors[tile->Collision];

				DBG_ASSERT(tile->Collision >= 0 && tile->Collision < MapTileCollisionType::COUNT);

				float r = collision_color.R / 255.0f;
				float g = collision_color.G / 255.0f;
				float b = collision_color.B / 255.0f;
				float a = collision_color.A / 255.0f;

				if (tile->FlipHorizontal == true)
				{
					float tmp = uv_left;
					uv_left = uv_right;
					uv_right = tmp;
				}

				if (tile->FlipVertical == true)
				{
					float tmp = uv_top;
					uv_top = uv_bottom;
					uv_bottom = tmp;
				}

				*(collision_buffer++) = left;				*(collision_buffer++) = top;				*(collision_buffer++) = layer_depth;
				*(collision_buffer++) = uv_left;			*(collision_buffer++) = uv_top;
				*(collision_buffer++) = r;					*(collision_buffer++) = g;					*(collision_buffer++) = b;					*(collision_buffer++) = a;

				*(collision_buffer++) = left;				*(collision_buffer++) = bottom;				*(collision_buffer++) = layer_depth;
				*(collision_buffer++) = uv_left;			*(collision_buffer++) = uv_bottom;
				*(collision_buffer++) = r;					*(collision_buffer++) = g;					*(collision_buffer++) = b;					*(collision_buffer++) = a;

				*(collision_buffer++) = right;				*(collision_buffer++) = bottom;				*(collision_buffer++) = layer_depth;
				*(collision_buffer++) = uv_right;			*(collision_buffer++) = uv_bottom;
				*(collision_buffer++) = r;					*(collision_buffer++) = g;					*(collision_buffer++) = b;					*(collision_buffer++) = a;

				*(collision_buffer++) = right;				*(collision_buffer++) = top;				*(collision_buffer++) = layer_depth;
				*(collision_buffer++) = uv_right;			*(collision_buffer++) = uv_top;
				*(collision_buffer++) = r;					*(collision_buffer++) = g;					*(collision_buffer++) = b;					*(collision_buffer++) = a;
			}
		}
	}
		
	// Unlock geometry.
	m_map_opaque_geometry->Unlock_Vertices(map_opaque_buffer);
	m_map_transparent_geometry->Unlock_Vertices(map_transparent_buffer);
	m_collision_geometry->Unlock_Vertices(collision_buffer);

	m_dirty = false;
	m_map->Mark_Collision_Dirty();
}	

struct MapLayerStrip
{
	MapTileCollisionType::Type	type;
	u16							frame;
	Rect2D						area;
	bool						merged;
};

void MapLayer::Create_Collision()
{
	// Rather than adding each individual tile to the collision manager (which would be stupidly expensive for no good reason), we instead
	// do the following algorithm to simplify the collision adde;d
	//
	// We go down the y axis from left to rightand create rectangular strips of collision.
	// Once we have the strips we then try to merge ones of the same height that are next to each other together horizontally.
	
	CollisionManager* col_mgr = CollisionManager::Get();
	float cell_width  = (float)m_map->Get_Tile_Width();
	float cell_height = (float)m_map->Get_Tile_Height();
	Atlas* tileset = m_map->Get_Tileset()->Get();

	std::vector<MapLayerStrip> strips;

	int total_solid_tiles = 0;

	// Create vertical strips.
	for (int x = 0; x < m_width; x++)
	{
		MapLayerStrip* strip = NULL;

		for (int y = 0; y < m_height; y++)
		{
			MapTile* tile = Get_Tile(x, y);			

			// Solid?
			if (tile->Collision != MapTileCollisionType::None && 
				tile->CollisionFrame != 0xFFFF)
			{
				total_solid_tiles++;

				Rect2D bounds = tileset->Get_Frame_By_Index(tile->CollisionFrame)->PixelBounds;

				if (tile->FlipHorizontal)
				{
					bounds.X = cell_width - bounds.X - bounds.Width;
				}
				if (tile->FlipVertical)
				{
					bounds.Y = cell_height - bounds.Y - bounds.Height;
				}

				Rect2D area = Rect2D((x * cell_width) + bounds.X, (y * cell_height) + bounds.Y, bounds.Width, bounds.Height);

				//DBG_LOG("Bounds[%i,%i] = %f,%f,%f,%f", x, y, bounds.X, bounds.Y, bounds.Width, bounds.Height);

				// If collision is different, or area width is different, end current stirp.
				if (strip != NULL &&
					(
						tile->Collision != strip->type ||
						tile->CollisionFrame != strip->frame ||
						area.X != strip->area.X ||
						area.Width != strip->area.Width
					))
				{
					strip = NULL;
				}

				// Continuing strip?
				if (strip != NULL && 
					strip->area.Y + strip->area.Height == area.Y)
				{
					strip->area.Height += cell_height;
				}

				// New strip.
				else
				{
					MapLayerStrip ns;
					ns.frame	= tile->CollisionFrame;
					ns.type		= tile->Collision;
					ns.area		= area;
					ns.merged	= false;
					strips.push_back(ns);

					strip = &strips.back();
				}
			}

			// Non-solid?
			else
			{
				// End strip.
				strip = NULL;
			}
		}
	}
	
	int total_unmerged_strips = strips.size();

	// Merge strips horizontally.
	for (unsigned int i = 0; i < strips.size(); i++)
	{
		MapLayerStrip& primary = strips.at(i);

		if (primary.merged == true)
			continue;

		for (unsigned int j = 0; j < strips.size(); j++)
		{
			MapLayerStrip& secondary = strips.at(j);

			if (secondary.merged == true)
				continue;

			if (j == i)
				continue;

			if (primary.area.X + primary.area.Width == secondary.area.X &&
				primary.area.Y == secondary.area.Y &&
				primary.area.Height == secondary.area.Height &&
				secondary.type == primary.type)
			{
				primary.area.Width += secondary.area.Width;
				secondary.merged = true;
			}
		}
	}

	// Add to collision manager.	
	int total_merged_strips = 0;

	for (unsigned int i = 0; i < strips.size(); i++)
	{
		MapLayerStrip& primary = strips.at(i);

		if (primary.merged == true)
			continue;

		Vector3 position = Vector3
		(
			primary.area.X,
			primary.area.Y,
			0.0f
		);

		Rect2D area = Rect2D
		(
			0.0f,
			0.0f,
			primary.area.Width,
			primary.area.Height
		);

		// Don't care about remanents of merged strips.
		if (primary.area.Width == 0.0f ||
			primary.area.Height == 0.0f)
		{
			continue;
		}
		
		CollisionGroup::Type collides_with;
		CollisionType::Type type;

		switch (primary.type)
		{
		case MapTileCollisionType::Solid:	
			{
				type = CollisionType::Solid;
				collides_with = CollisionGroup::All;
				break;
			}		
		case MapTileCollisionType::Player_Only:
			{
				type = CollisionType::Solid;
				collides_with = CollisionGroup::Player;
				break;
			}
		case MapTileCollisionType::Enemy_Only:
			{
				type = CollisionType::Solid;
				collides_with = CollisionGroup::Enemy;
				break;
			}
		default:
			{
				continue;
			}
		}

		col_mgr->Create_Handle(
			type,
			CollisionShape::Rectangle,
			CollisionGroup::Environment,
			collides_with,
			area,
			position,
			position,
			true);

		total_merged_strips++;
	}
		
	DBG_LOG("[Map Collision] Merged %i solid tiles into %i merged rectangles.", total_solid_tiles, total_merged_strips);
}

void MapLayer::Tick(const FrameTime& time)
{
	for (std::vector<Animation*>::iterator iter = m_tile_animations.begin(); iter != m_tile_animations.end(); iter++)
	{
		Animation* anim = *iter;
		anim->Advance(time);
	}
}
// Adds all draw instances for this drawable.
void MapLayer::Add_Draw_Instances(const FrameTime& time, std::vector<DrawInstance>& instances) 
{
	// Opaque tiles.
	DrawInstance instance;
	instance.draw_depth = Get_Draw_Depth();
	instance.draw_index = 0;
	instance.drawable = this;
	instance.transparent = false;
	instances.push_back(instance);

	// Transparent tiles.
	instance.draw_depth = Get_Draw_Depth();
	instance.draw_index = 1;
	instance.drawable = this;
	instance.transparent = true;
	instances.push_back(instance);
}

void MapLayer::Draw(const FrameTime& time, RenderPipeline* pipeline, const DrawInstance& instance)
{
	AtlasHandle* tileset = m_map->Get_Tileset();
	if (tileset == NULL)
	{
		return;
	}

	if (m_dirty == true)
	{
		Regenerate();
	}

	int tile_width = m_map->Get_Tile_Width();
	int tile_height = m_map->Get_Tile_Height();

	float depth = (m_layer_index * MAP_LAYER_DEPTH);

	Renderer* renderer = Renderer::Get();

	if (instance.draw_index  == 0 && 
		m_map->m_draw_active_layer_tint == true && 
		m_map->m_draw_active_layer_tint_index == m_layer_index)
	{
		bool bDepthTestOn = renderer->Get_Depth_Test();
		bool bDepthWriteOn = renderer->Get_Depth_Write();

		renderer->Set_Depth_Test(false);
		renderer->Set_Depth_Write(false);

		PrimitiveRenderer pr;
		pr.Draw_Solid_Quad(Rect2D(0, 0, m_width * tile_width, m_height * tile_height), Color(255, 255, 255, 200));

		renderer->Set_Depth_Test(bDepthTestOn);
		renderer->Set_Depth_Write(bDepthWriteOn);
	}

	// Draw animated tiles seperately.
	// TODO: For speed, we could not render off-screen tiles here, but it's relatively pointless seeing
	//		 as maps tend to have very few animated tiles on them at the moment.
	AtlasRenderer atlas_renderer(tileset);

	// Draw the static plane.
	Geometry* geometry = m_map_opaque_geometry;
	if (instance.draw_index == 1)
	{
		geometry = m_map_transparent_geometry;
	}

	if (geometry != NULL)
	{
		// Render all batched drawing.
		pipeline->Flush_Batches();

		RenderPipeline_Shader* shader = pipeline->Get_Active_Shader();
		if (shader != NULL)
		{
			shader->Shader_Program->Get()->Bind_Texture("g_texture", 0);
			shader->Shader_Program->Get()->Bind_Bool("g_texture_enabled", true);	
			shader->Shader_Program->Get()->Bind_Vector("g_draw_color", m_map->Get_Color_Tint());	
		}

		renderer->Set_Alpha_Test(true);
		renderer->Set_Blend(true);
		renderer->Set_Blend_Function(RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);
		renderer->Bind_Texture(tileset->Get()->Get_Texture(0)->TexturePtr, 0);

		static int render_layer_index = -1; //useful hook to change while debugging.
		if (render_layer_index == -1 || render_layer_index == m_layer_index)
			geometry->Render();

		//shader->Shader_Program->Get()->Bind_Vector("g_draw_color", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	}
	
	// Assume all animated are opaque, blah.
	if (instance.draw_index == 0)
	{
		atlas_renderer.Begin_Batch();

		Rect2D bounds = pipeline->Get_Active_Camera()->Get_Bounding_Viewport();

		for (std::vector<AnimatedTile>::iterator iter = m_animated_tiles.begin(); iter != m_animated_tiles.end(); iter++)
		{
			AnimatedTile& anim = *iter;
			MapTile* tile = anim.Tile;
			AtlasFrame* frame = tileset->Get()->Get_Frame_By_Index(tile->TileAnimation->Get_Frame());

			Rect2D position
			(
				anim.X * tile_width,
				anim.Y * tile_height,
				tile_width,
				tile_height
			);

			if (bounds.Intersects(position))
			{
				atlas_renderer.Draw_Frame(frame, position, depth, tile->TileColor, tile->FlipHorizontal, tile->FlipVertical);
			}
		}

		atlas_renderer.End_Batch();
	}
}

void MapLayer::Draw_Collision(const FrameTime& time, RenderPipeline* pipeline)
{
	if (m_dirty == true)
	{
		Regenerate();
	}

	Renderer* renderer = Renderer::Get();
	AtlasHandle* tileset = m_map->Get_Tileset();

	// Draw the static plane.
	if (m_collision_geometry >= 0 && m_collision_visible == true)
	{
		renderer->Set_Alpha_Test(true);
		renderer->Set_Blend(true);
		renderer->Set_Blend_Function(RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);
		renderer->Bind_Texture(tileset->Get()->Get_Texture(0)->TexturePtr, 0);

		m_collision_geometry->Render();
	}
}

Map::Map()
	: m_width(0)
	, m_height(0)
	, m_depth(0)
	, m_layers(NULL)
	, m_tile_width(0)
	, m_tile_height(0)
	, m_tileset(NULL)
	, m_collision_dirty(false)
	, m_color_tint(1.0f, 1.0f, 1.0f, 1.0f)
	, m_preview_pixmap(NULL)
	, m_preview_texture(NULL)
	, m_owns_preview_pixmap(false)
	, m_draw_active_layer_tint(false)
	, m_dirty_time(0.0f)
	, m_regenerate_defer_interval(0.0f)
{
	Resize(DEFAULT_MAP_WIDTH, DEFAULT_MAP_HEIGHT, DEFAULT_MAP_DEPTH);
}

Map::~Map()
{
	SAFE_DELETE(m_preview_texture);
	SAFE_DELETE(m_preview_pixmap);
	Free_Layers(m_layers);
}

void Map::Set_Active_Layer_Tint(bool bTinted, int activeLayerIndex)
{
	m_draw_active_layer_tint = bTinted;
	m_draw_active_layer_tint_index = activeLayerIndex;
}

void Map::Set_Regenerate_Defer_Time(float time)
{
	m_regenerate_defer_interval = time;
}

Vector4 Map::Get_Color_Tint()
{
	return m_color_tint;
}

void Map::Set_Color_Tint(Vector4 tint)
{
	m_color_tint = tint;
}

void Map::Mark_Collision_Dirty()
{
	m_collision_dirty = true;
	m_dirty_time = 0.0f;
}

void Map::Free_Layers(MapLayer** layers)
{
	if (layers != NULL)
	{
		DBG_LOG("Freeing map layers ...");
		for (int i = 0; i < m_depth; i++)
		{		
			if (layers[i] != NULL)
			{
				GameEngine::Get()->Get_Scene()->Remove_Drawable(layers[i]);
				SAFE_DELETE(layers[i]);
			}
		}
		SAFE_DELETE_ARRAY(layers);
	}
}

void Map::Create_Collision()
{
	for (int i = 0; i < m_depth; i++)
	{
		m_layers[i]->Create_Collision();
	}

	m_collision_dirty = false;
}

void Map::Tick(const FrameTime& time)
{
	for (int i = 0; i < m_depth; i++)
	{
		m_layers[i]->Tick(time);
	}
	
	// Should only be executed by editor, slow as fuck typically!
	if (m_collision_dirty)
	{
		m_dirty_time += time.Get_Delta_Seconds();
	}

	if (m_collision_dirty == true && (m_regenerate_defer_interval == 0.0f || m_dirty_time > m_regenerate_defer_interval))
	{
		DBG_LOG("Map collision is dirty, regenerating!");
		double start = Platform::Get()->Get_Ticks();

		CollisionManager::Get()->Reset();
		PathManager::Get()->Reset();
		Create_Collision();

		m_collision_dirty = false;
		m_dirty_time = 0.0f;

		double elapsed = Platform::Get()->Get_Ticks() - start;
		DBG_LOG("Collision generation took %.2f ms.", elapsed);
	}

	for (std::vector<Animation>::iterator iter = m_all_animations.begin(); iter != m_all_animations.end(); iter++)
	{
		Animation& anim = *iter;
		anim.Advance(time);
	}
}

void Map::Mark_Dirty()
{
	for (int i = 0; i < m_depth; i++)
	{
		m_layers[i]->Mark_Dirty();
	}
	m_dirty_time = 0.0f;
}

void Map::Resize(int width, int height, int depth, bool persist)
{
	DBG_ASSERT(width > 0 && height > 0 && depth > 0);

	// Already correct size?
	if (m_width == width && m_height == height && m_depth == depth)
	{
		return;
	}

	// Store old layer incase of persitance.
	MapLayer** old_layers = m_layers;
	int min_width = Min(m_width, width);
	int min_height = Min(m_height, height);
	int min_depth = Min(m_depth, depth);

	// Initialize new layers.
	m_width = width;
	m_height = height;
	m_depth = depth;
	m_layers = new MapLayer*[m_depth];
	
	DBG_LOG("Creating map layers ...");
	for (int i = 0; i < m_depth; i++)
	{
		m_layers[i] = new MapLayer(this, i, m_width, m_height);
		GameEngine::Get()->Get_Scene()->Add_Drawable(m_layers[i]);
	}

	// Copy old data into new layers if required.
	if (persist == true)
	{
		for (int z = 0; z < min_depth; z++)
		{			
			MapLayer* old_layer = old_layers[z];
			MapLayer* new_layer = m_layers[z];

			for (int y = 0; y < min_height; y++)
			{
				for (int x = 0; x < min_width; x++)
				{
					MapTile* old_tile = old_layer->Get_Tile(x, y);
					MapTile* new_tile = new_layer->Get_Tile(x, y);

					*new_tile = *old_tile;
					new_tile->TileAnimation = NULL;
				}
			}
		}
	}

	// Free old layers.
	Free_Layers(old_layers);
}

int Map::Get_Width()
{
	return m_width;
}

int Map::Get_Height()
{
	return m_height;
}

int Map::Get_Depth()
{
	return m_depth;
}

AtlasHandle* Map::Get_Tileset()
{
	return m_tileset;
}

int Map::Get_Tile_Width()
{
	return m_tile_width;
}

int Map::Get_Tile_Height()
{
	return m_tile_height;
}

Vector2 Map::Get_Tile_Size()
{
	return Vector2(m_tile_width, m_tile_height);
}

MapTile* Map::Get_Tile(int x, int y, int z)
{
	DBG_ASSERT(x >= 0 && x < m_width);
	DBG_ASSERT(y >= 0 && y < m_height);
	DBG_ASSERT(z >= 0 && z < m_depth);

	MapTile* tile = m_layers[z]->Get_Tile(x, y);
	return tile;
}

MapLayer* Map::Get_Layer(int z)
{
	DBG_ASSERT(z >= 0 && z < m_depth);

	return m_layers[z];
}

void Map::Set_Tileset(AtlasHandle* atlas)
{
	Mark_Dirty();
	m_tileset = atlas;

	AtlasFrame* frame	= atlas->Get()->Get_Frame_By_Index(0);
	m_tile_width		= (int)frame->Rect.Width;
	m_tile_height		= (int)frame->Rect.Height;

	m_all_animations.clear();

	std::vector<AtlasAnimation*>& anims = atlas->Get()->Get_Animations_List();
	for (std::vector<AtlasAnimation*>::iterator iter = anims.begin(); iter != anims.end(); iter++)
	{
		AtlasAnimation* anim = *iter;
		m_all_animations.push_back(Animation(anim));
	}
}

std::vector<Animation> Map::Get_All_Tile_Animations()
{
	return m_all_animations;
}

MapFileHeaderBlock& Map::Get_Map_Header()
{
	return m_header;
}

void Map::Set_Map_Header(MapFileHeaderBlock block)
{
	m_header = block;
}

void Map::Set_Preview_Pixmap(Pixelmap* pix, bool owns_pixmap)
{
	if (m_preview_texture != NULL)
	{
		SAFE_DELETE(m_preview_texture);
	}
	if (m_owns_preview_pixmap)
	{
		if (m_preview_pixmap != NULL)
		{
			SAFE_DELETE(m_preview_pixmap);
		}
	}
	m_preview_pixmap = pix;
	if (pix != NULL)
	{
		m_preview_texture = Renderer::Get()->Create_Texture(pix, TextureFlags::PersistSourceData);
	}
	m_owns_preview_pixmap = owns_pixmap;
}

Pixelmap* Map::Get_Preview_Pixmap()
{
	return m_preview_pixmap;
}

Texture* Map::Get_Preview_Image()
{
	return m_preview_texture;
}

void Map::Apply_Header_Changes()
{
	Scene* scene = GameEngine::Get()->Get_Scene();

	if (m_header.Tileset != m_tileset && m_header.Tileset != NULL)
	{
		Set_Tileset(m_header.Tileset);
	}
	if (m_header.Width != m_width || m_header.Height != m_height)
	{
		Resize(m_header.Width, m_header.Height, m_depth, true);
		scene->Set_Boundries(Rect2D(0.0f, 0.0f, (float)Get_Width() * Get_Tile_Width(), (float)Get_Height() * Get_Tile_Height()));
	}
}