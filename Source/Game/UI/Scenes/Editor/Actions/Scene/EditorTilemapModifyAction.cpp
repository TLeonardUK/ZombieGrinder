// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Editor/Actions/Scene/EditorTilemapModifyAction.h"
#include "Game/Game/EditorManager.h"
#include "Game/Runner/Game.h"

#include "Engine/Scene/Map/Map.h"

struct FloodFillPoint
{
public:
	int Tilemap_X;
	int Tilemap_Y;
	int Tileset_X;
	int Tileset_Y;

public:
	FloodFillPoint(int x, int y, int tileset_x, int tileset_y)
		: Tilemap_X(x)
		, Tilemap_Y(y)
		, Tileset_X(tileset_x)
		, Tileset_Y(tileset_y)
	{
	}
};

EditorTilemapModifyAction::EditorTilemapModifyAction(NetPacket_C2S_ModifyTilemap* action)
	: m_action(*action)
{
}

const char* EditorTilemapModifyAction::Get_Name()
{
	return "Tilemap Modification";
}

void EditorTilemapModifyAction::Undo()
{
	DBG_LOG("Performing undo operation for tilemap modification.");
	
	Map* map = Game::Get()->Get_Map();

	int index = 0;
	for (int x = 0; x < m_original_tiles_bounds.Width; x++)
	{				
		for (int y = 0; y < m_original_tiles_bounds.Height; y++, index++)
		{
			int tile_x = (int)(m_original_tiles_bounds.X + x);
			int tile_y = (int)(m_original_tiles_bounds.Y + y);

			MapTile* tile = map->Get_Tile(tile_x, tile_y, m_action.Layer_Index);
			
			*tile = m_original_tiles.at(index);
			tile->TileAnimation = NULL;
		}
	}
	
	map->Get_Layer(m_action.Layer_Index)->Mark_Dirty();
}

void EditorTilemapModifyAction::Do()
{
	NetPacket_C2S_ModifyTilemap* packet = &m_action;

	DBG_LOG("Performing tilemap action %i.", packet->Action);

	Map* map = Game::Get()->Get_Map();
	Atlas* tileset = map->Get_Tileset()->Get();
	AtlasFrame* frame = tileset->Get_Frame_By_Index(0);

	// Store information for undo.
	switch (packet->Action)
	{
	case ModifyTilemapAction::Paste:
		{
			m_original_tiles_bounds = Rect2D(0, 0, map->Get_Width(), map->Get_Height());
			break;
		}
	case ModifyTilemapAction::Eraser:
	case ModifyTilemapAction::Pencil:
		{
			m_original_tiles_bounds = packet->Tilemap_Selection;
			m_original_tiles_bounds = m_original_tiles_bounds.ClampInside(Rect2D(0, 0, map->Get_Width(), map->Get_Height()));
			break;
		}
	case ModifyTilemapAction::Flood:
		{
			m_original_tiles_bounds = Rect2D(0, 0, map->Get_Width(), map->Get_Height());
			break;
		}
	case ModifyTilemapAction::Circle:
		{
			m_original_tiles_bounds = packet->Tilemap_Selection.Inflate(packet->Radius * 3.0f, packet->Radius * 3.0f);
			m_original_tiles_bounds = m_original_tiles_bounds.ClampInside(Rect2D(0, 0, map->Get_Width(), map->Get_Height()));
			break;
		}
	}

	m_original_tiles.reserve((int)(m_original_tiles_bounds.Width * m_original_tiles_bounds.Height));
	for (int x = 0; x < m_original_tiles_bounds.Width; x++)
	{				
		for (int y = 0; y < m_original_tiles_bounds.Height; y++)
		{
			int tile_x = (int)(m_original_tiles_bounds.X + x);
			int tile_y = (int)(m_original_tiles_bounds.Y + y);

			MapTile* tile = map->Get_Tile(tile_x, tile_y, packet->Layer_Index);

			m_original_tiles.push_back(*tile);
		}
	}

	// Perform action.
	switch (packet->Action)
	{
	case ModifyTilemapAction::Paste:
		{
			DBG_LOG("Performing tilemap paste action (Tilemap Region:%f,%f,%f,%f).",
				packet->Tilemap_Selection.X, packet->Tilemap_Selection.Y, packet->Tilemap_Selection.Width, packet->Tilemap_Selection.Height);

			for (int y = 0; y < packet->Tilemap_Selection.Height; y++)
			{
				for (int x = 0; x < packet->Tilemap_Selection.Width; x++)
				{
					ModifyTilemapTileState& state_tile = packet->Tiles.at((y * (int)packet->Tilemap_Selection.Width) + x);
					
					int tile_x = (int)(packet->Tilemap_Selection.X + x);
					int tile_y = (int)(packet->Tilemap_Selection.Y + y);

					if (tile_x >= 0 && tile_y >= 0 && tile_x < map->Get_Width() && tile_y < map->Get_Height())
					{
						MapTile* tile = map->Get_Tile(tile_x, tile_y, packet->Layer_Index);
						
						if (packet->Mode != ModifyTilemapMode::Collision)
						{
							tile->Frame				= state_tile.Frame;
							tile->TileColor			= state_tile.Tile_Color;
							tile->FlipHorizontal	= state_tile.Flip_Horizontal;
							tile->FlipVertical		= state_tile.Flip_Vertical;
						}
						else
						{
							tile->CollisionFrame	= state_tile.Collision_Frame;
							tile->Collision			= (MapTileCollisionType::Type)state_tile.Collision;
							tile->FlipHorizontal	= state_tile.Flip_Horizontal;
							tile->FlipVertical		= state_tile.Flip_Vertical;
						}
					}					
				}
			}

			map->Get_Layer(packet->Layer_Index)->Mark_Dirty();
		}
		break;

	case ModifyTilemapAction::Eraser:
		{
			DBG_LOG("Performing tilemap erase action (Tilemap Region:%f,%f,%f,%f, Tileset Region:%f,%f,%f,%f).",
				packet->Tilemap_Selection.X, packet->Tilemap_Selection.Y, packet->Tilemap_Selection.Width, packet->Tilemap_Selection.Height,
				packet->Tileset_Selection.X, packet->Tileset_Selection.Y, packet->Tileset_Selection.Width, packet->Tileset_Selection.Height);

			// Apply action.
			for (int x = 0; x < packet->Tilemap_Selection.Width; x++)
			{				
				for (int y = 0; y < packet->Tilemap_Selection.Height; y++)
				{
					int tile_x = (int)(packet->Tilemap_Selection.X + x);
					int tile_y = (int)(packet->Tilemap_Selection.Y + y);

					MapTile* tile = map->Get_Tile(tile_x, tile_y, packet->Layer_Index);
					
					if (packet->Mode != ModifyTilemapMode::Collision)
					{
						tile->Frame				= -1;
					}
					else
					{
						tile->CollisionFrame	= -1;
						tile->Collision			= MapTileCollisionType::None;
					}
				}
			}

			map->Get_Layer(packet->Layer_Index)->Mark_Dirty();

			break;
		}
	case ModifyTilemapAction::Pencil:
		{
			DBG_LOG("Performing tilemap pencil action (Tilemap Region:%f,%f,%f,%f, Tileset Region:%f,%f,%f,%f).",
				packet->Tilemap_Selection.X, packet->Tilemap_Selection.Y, packet->Tilemap_Selection.Width, packet->Tilemap_Selection.Height,
				packet->Tileset_Selection.X, packet->Tileset_Selection.Y, packet->Tileset_Selection.Width, packet->Tileset_Selection.Height);

			for (int x = 0; x < packet->Tilemap_Selection.Width; x++)
			{				
				for (int y = 0; y < packet->Tilemap_Selection.Height; y++)
				{
					int tile_x = (int)(packet->Tilemap_Selection.X + x);
					int tile_y = (int)(packet->Tilemap_Selection.Y + y);
					int tileset_x = (int)(packet->Tileset_Selection.X + (x % (int)packet->Tileset_Selection.Width));
					int tileset_y = (int)(packet->Tileset_Selection.Y + (y % (int)packet->Tileset_Selection.Height));

					MapTile* tile = map->Get_Tile(tile_x, tile_y, packet->Layer_Index);

					int tile_offset = (int)((tileset_y * frame->GridOrigin.Width) + tileset_x);

					if (packet->Mode != ModifyTilemapMode::Collision)
					{
						tile->Frame				= tile_offset;
						tile->TileColor			= packet->Tile_Color;
						tile->FlipHorizontal	= packet->Flip_Horizontal;
						tile->FlipVertical		= packet->Flip_Vertical;
					}
					else
					{
						tile->CollisionFrame	= tile_offset;
						tile->Collision			= (MapTileCollisionType::Type)packet->Collision;
					}
				}
			}

			map->Get_Layer(packet->Layer_Index)->Mark_Dirty();

			break;
		}
	case ModifyTilemapAction::Circle:
		{
			DBG_LOG("Performing tilemap circle action (Tilemap Region:%f,%f,%f,%f, Tileset Region:%f,%f,%f,%f).",
				packet->Tilemap_Selection.X, packet->Tilemap_Selection.Y, packet->Tilemap_Selection.Width, packet->Tilemap_Selection.Height,
				packet->Tileset_Selection.X, packet->Tileset_Selection.Y, packet->Tileset_Selection.Width, packet->Tileset_Selection.Height);

			int center_x = (int)(packet->Tilemap_Selection.X + (packet->Tilemap_Selection.Width / 2));
			int center_y = (int)(packet->Tilemap_Selection.Y + (packet->Tilemap_Selection.Height / 2));

			for (int x = 0; x < m_original_tiles_bounds.Width; x++)
			{				
				for (int y = 0; y < m_original_tiles_bounds.Height; y++)
				{
					int tile_x = (int)(m_original_tiles_bounds.X + x);
					int tile_y = (int)(m_original_tiles_bounds.Y + y);
					int tileset_x = (int)(packet->Tileset_Selection.X + (x % (int)packet->Tileset_Selection.Width));
					int tileset_y = (int)(packet->Tileset_Selection.Y + (y % (int)packet->Tileset_Selection.Height));

					MapTile* tile = map->Get_Tile(tile_x, tile_y, packet->Layer_Index);

					int tile_offset = (int)((tileset_y * frame->GridOrigin.Width) + tileset_x);

					float diff = (Vector2(tile_x, tile_y) - Vector2(center_x, center_y)).Length();

					if (diff <= packet->Radius)
					{
						if (packet->Mode != ModifyTilemapMode::Collision)
						{
							tile->Frame				= tile_offset;
							tile->TileColor			= packet->Tile_Color;
							tile->FlipHorizontal	= packet->Flip_Horizontal;
							tile->FlipVertical		= packet->Flip_Vertical;
						}
						else
						{
							tile->CollisionFrame	= tile_offset;
							tile->Collision			= (MapTileCollisionType::Type)packet->Collision;
						}
					}
				}
			}

			map->Get_Layer(packet->Layer_Index)->Mark_Dirty();

			break;
		}
	case ModifyTilemapAction::Flood:
		{
			DBG_LOG("Performing tilemap flood fill action (Tilemap Region:%f,%f,%f,%f, Tileset Region:%f,%f,%f,%f).",
				packet->Tilemap_Selection.X, packet->Tilemap_Selection.Y, packet->Tilemap_Selection.Width, packet->Tilemap_Selection.Height,
				packet->Tileset_Selection.X, packet->Tileset_Selection.Y, packet->Tileset_Selection.Width, packet->Tileset_Selection.Height);

			// Grab seed point.
			int seed_tile_frame = (int)map->Get_Tile((int)packet->Tilemap_Selection.X, (int)packet->Tilemap_Selection.Y, (int)packet->Layer_Index)->Frame;
			int seed_tile_collision = (int)map->Get_Tile((int)packet->Tilemap_Selection.X, (int)packet->Tilemap_Selection.Y, (int)packet->Layer_Index)->Collision;
			int map_width = map->Get_Width();
			int map_height = map->Get_Height();

			// Create grid of flags
			bool* tile_flags = new bool[map_width * map_height];
			//for (int x = 0; x < map_width * map_height; x++)
			//	tile_flags[x] = false;
			memset(tile_flags, 0, map_width * map_height * sizeof(bool));

			// Non-recursive flood filling!
			std::vector<FloodFillPoint> stack;
			stack.push_back(FloodFillPoint((int)packet->Tilemap_Selection.X, (int)packet->Tilemap_Selection.Y,
										   (int)packet->Tileset_Selection.X, (int)packet->Tileset_Selection.Y));
			while (stack.size() > 0)
			{
				FloodFillPoint point = stack.at(stack.size() - 1);
				stack.pop_back();
				//stack.erase(stack.begin());

				// Ignore points outside the map (would be more efficient to just not push them!)
				if (point.Tilemap_X < 0 || point.Tilemap_Y < 0 || 					
					point.Tilemap_X >= map_width || point.Tilemap_Y >= map_height)
				{
					continue;
				}

				int offset = (point.Tilemap_Y * map_width) + point.Tilemap_X;
				DBG_ASSERT(offset >= 0 && offset < map_width * map_height);

				bool& flag = tile_flags[offset];
				MapTile* tile = map->Get_Tile(point.Tilemap_X, point.Tilemap_Y, packet->Layer_Index);

				if (flag == false && ((packet->Mode == ModifyTilemapMode::Tilemap && tile->Frame == seed_tile_frame) || (packet->Mode == ModifyTilemapMode::Collision && tile->Collision == seed_tile_collision)))
				{
					int tileset_x			= point.Tileset_X;
					int tileset_y			= point.Tileset_Y;

					if (tileset_x < packet->Tileset_Selection.X)
						tileset_x = (int)(packet->Tileset_Selection.X + packet->Tileset_Selection.Width) - 1;
					if (tileset_y < packet->Tileset_Selection.Y)
						tileset_y = (int)(packet->Tileset_Selection.Y + packet->Tileset_Selection.Height) - 1;
					if (tileset_x >= packet->Tileset_Selection.X + packet->Tileset_Selection.Width)
						tileset_x = (int)packet->Tileset_Selection.X;
					if (tileset_y >= packet->Tileset_Selection.Y + packet->Tileset_Selection.Height)
						tileset_y = (int)packet->Tileset_Selection.Y;		
					
					int tile_offset			= (int)((tileset_y * frame->GridOrigin.Width) + tileset_x);
					
					if (packet->Mode != ModifyTilemapMode::Collision)
					{
						tile->Frame				= tile_offset;
						tile->TileColor			= packet->Tile_Color;
						tile->FlipHorizontal	= packet->Flip_Horizontal;
						tile->FlipVertical		= packet->Flip_Vertical;
					}
					else
					{
						tile->CollisionFrame = tile_offset;
						tile->Collision = (MapTileCollisionType::Type)packet->Collision;
					}

					flag = true;

					stack.push_back(FloodFillPoint(point.Tilemap_X + 1, point.Tilemap_Y, tileset_x + 1, tileset_y));
					stack.push_back(FloodFillPoint(point.Tilemap_X - 1, point.Tilemap_Y, tileset_x - 1, tileset_y));
					stack.push_back(FloodFillPoint(point.Tilemap_X, point.Tilemap_Y + 1, tileset_x, tileset_y + 1));
					stack.push_back(FloodFillPoint(point.Tilemap_X, point.Tilemap_Y - 1, tileset_x, tileset_y - 1));
				}
			}

			delete[] tile_flags;

			map->Get_Layer(packet->Layer_Index)->Mark_Dirty();

			break;
		}
	}

	if (packet->Mode == ModifyTilemapMode::Collision)
	{	
		map->Mark_Collision_Dirty();
	}
}
