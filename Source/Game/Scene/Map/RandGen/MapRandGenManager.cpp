// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Map/RandGen/MapRandGenManager.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Textures/PixelmapFactory.h"

#include "Game/Scene/Actors/ScriptedActor.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Engine/GameEngine.h"

#include "Game/Runner/Game.h"
#include "Game/Runner/GameOptions.h"
#include "Game/Scene/GameScene.h"
#include "Game/Scene/Actors/ScriptedActor.h"

const char* MapRandGenRoomType::Mnemonics[MapRandGenRoomType::COUNT] = {
	"Arena",
	"Boss",
	"Corridoor",
	"Start",
	"End",
	"Resupply",
	"Treasure",
	"Boss_Treasure"
};

MapRandGenManager::MapRandGenManager()
	: m_rand(0)
{
	// Arena room rules.
	m_room_type_rules[MapRandGenRoomType::Arena].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::NotEqual, MapRandGenRoomRuleType::AbsoluteOffsetFromRootFirst, 0));
	m_room_type_rules[MapRandGenRoomType::Arena].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::NotEqual, MapRandGenRoomRuleType::AbsoluteOffsetFromRootLast, 0));
	m_room_type_rules[MapRandGenRoomType::Arena].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::NotDuplicateRoom, 0));

	// Boss room rules.
	m_room_type_rules[MapRandGenRoomType::Boss].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::NotEqual, MapRandGenRoomRuleType::AbsoluteOffsetFromRootFirst, 0));
	m_room_type_rules[MapRandGenRoomType::Boss].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::NotEqual, MapRandGenRoomRuleType::AbsoluteOffsetFromRootLast, 0));
	m_room_type_rules[MapRandGenRoomType::Boss].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::AbsoluteOffsetFromRootLast, 2));
	m_room_type_rules[MapRandGenRoomType::Boss].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::LimitedCount, 1));
	m_room_type_rules[MapRandGenRoomType::Boss].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::ForceRoomType, 0));

	// Corridoor room rules.
	m_room_type_rules[MapRandGenRoomType::Corridoor].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::NotEqual, MapRandGenRoomRuleType::AbsoluteOffsetFromRootFirst, 0));
	m_room_type_rules[MapRandGenRoomType::Corridoor].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::NotEqual, MapRandGenRoomRuleType::AbsoluteOffsetFromRootLast, 0));
	m_room_type_rules[MapRandGenRoomType::Corridoor].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::NotDuplicateRoom, 0));

	// Start room rules.
	m_room_type_rules[MapRandGenRoomType::Start].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::AbsoluteOffsetFromRootFirst, 0));
	m_room_type_rules[MapRandGenRoomType::Start].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::LimitedCount, 1));

	// End room rules.
	m_room_type_rules[MapRandGenRoomType::End].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::AbsoluteOffsetFromRootLast, 0));
	m_room_type_rules[MapRandGenRoomType::End].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::LimitedCount, 1));

	// Resupply room rules.
	m_room_type_rules[MapRandGenRoomType::Resupply].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::NotEqual, MapRandGenRoomRuleType::AbsoluteOffsetFromRootFirst, 0));
	m_room_type_rules[MapRandGenRoomType::Resupply].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::NotEqual, MapRandGenRoomRuleType::AbsoluteOffsetFromRootLast, 0));
	m_room_type_rules[MapRandGenRoomType::Resupply].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::NotEqual, MapRandGenRoomRuleType::OnRootPath, 0));
	m_room_type_rules[MapRandGenRoomType::Resupply].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::NotDuplicateRoom, 0));
	m_room_type_rules[MapRandGenRoomType::Resupply].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::NotDuplicateRoomType, 0));

	// Treasure room rules.
	m_room_type_rules[MapRandGenRoomType::Treasure].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::NotEqual, MapRandGenRoomRuleType::AbsoluteOffsetFromRootFirst, 0));
	m_room_type_rules[MapRandGenRoomType::Treasure].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::NotEqual, MapRandGenRoomRuleType::AbsoluteOffsetFromRootLast, 0));
	m_room_type_rules[MapRandGenRoomType::Treasure].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::LimitedCount, 2));
	m_room_type_rules[MapRandGenRoomType::Treasure].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::NotEqual, MapRandGenRoomRuleType::OnRootPath, 0));
	m_room_type_rules[MapRandGenRoomType::Treasure].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::NotDuplicateRoom, 0));
	m_room_type_rules[MapRandGenRoomType::Treasure].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::NotDuplicateRoomType, 0));
	m_room_type_rules[MapRandGenRoomType::Treasure].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::ForceRoomType, 0));
	m_room_type_rules[MapRandGenRoomType::Treasure].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::AbsoluteOffsetFromSubLast, 0));

	// Boss treasure room rules.
	m_room_type_rules[MapRandGenRoomType::Boss_Treasure].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::NotEqual, MapRandGenRoomRuleType::AbsoluteOffsetFromRootFirst, 0));
	m_room_type_rules[MapRandGenRoomType::Boss_Treasure].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::LimitedCount, 1));
	m_room_type_rules[MapRandGenRoomType::Boss_Treasure].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::ConnectedTo, MapRandGenRoomType::Boss));
	m_room_type_rules[MapRandGenRoomType::Boss_Treasure].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::AbsoluteOffsetFromRootLast, 1));
	m_room_type_rules[MapRandGenRoomType::Boss_Treasure].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::NotDuplicateRoom, 0));
	m_room_type_rules[MapRandGenRoomType::Boss_Treasure].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::NotDuplicateRoomType, 0));
	m_room_type_rules[MapRandGenRoomType::Boss_Treasure].push_back(MapRandGenRoomRule(MapRandGenRoomRuleOpType::Equal, MapRandGenRoomRuleType::ForceRoomType, 0));
}

void MapRandGenManager::LoadTemplate(MapFile* map_template)
{	
	std::vector<MapFileObjectBlock*> objects = map_template->Get_Blocks<MapFileObjectBlock>();

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	AtlasFrame* frame = map_template->Get_Header()->Tileset->Get()->Get_Frame_By_Index(0);
	int tile_width = (int)frame->Rect.Width;
	int tile_height = (int)frame->Rect.Height;

	m_tile_width = tile_width;
	m_tile_height = tile_height;

	int room_count = 0;
	int junction_count = 0;
	int object_count = 0;

	CVMLinkedSymbol* room_marker_symbol = vm->Find_Class("DC_Room_Marker");
	CVMLinkedSymbol* room_type_symbol = vm->Find_Variable(room_marker_symbol, "Room_Type");
	CVMLinkedSymbol* max_instance_symbol = vm->Find_Variable(room_marker_symbol, "Maximum_Instances");

	CVMLinkedSymbol* template_marker_symbol = vm->Find_Class("DC_Template_Marker");
	CVMLinkedSymbol* template_type_symbol = vm->Find_Variable(template_marker_symbol, "Template_Type");

	// Load room templates.
	for (std::vector<MapFileObjectBlock*>::iterator iter = objects.begin(); iter != objects.end(); iter++)
	{
		MapFileObjectBlock* obj = *iter;
		if (obj->Class_Name == "DC_Room_Marker")
		{
			ScriptedActor* marker = obj->Create_Instance();
			
			Rect2D world_bb = marker->Get_World_Bounding_Box();

			CVMValue type_value;
			vm->Get_Field(room_type_symbol, marker->Get_Script_Object(), type_value);

			CVMValue max_instance_value;
			vm->Get_Field(max_instance_symbol, marker->Get_Script_Object(), max_instance_value);

			MapRandGenRoomTemplate temp;
			temp.bounds			= world_bb;
			temp.tile_x			= (int)(world_bb.X / tile_width);
			temp.tile_y			= (int)(world_bb.Y / tile_height);
			temp.tile_width		= (int)(world_bb.Width / tile_width);
			temp.tile_height	= (int)(world_bb.Height / tile_height);
			temp.room_width		= (int)(temp.tile_width / template_room_tile_width);
			temp.room_height	= (int)(temp.tile_height / template_room_tile_height);
			temp.type			= (MapRandGenRoomType::Type)type_value.int_value;
			temp.rules			= m_room_type_rules[temp.type];
			temp.max_instances	= max_instance_value.int_value;
			temp.instance_count	= 0;

			DBG_LOG("Room Type=%i Max=%i", temp.type, temp.max_instances);

			SAFE_DELETE(marker);

			m_room_templates.push_back(temp);

			room_count++;
		}
	}

	// Load template templates.
	for (std::vector<MapFileObjectBlock*>::iterator iter = objects.begin(); iter != objects.end(); iter++)
	{
		MapFileObjectBlock* obj = *iter;
		if (obj->Class_Name == "DC_Template_Marker")
		{
			ScriptedActor* marker = obj->Create_Instance();

			Rect2D world_bb = marker->Get_World_Bounding_Box();

			CVMValue type_value;
			vm->Get_Field(template_type_symbol, marker->Get_Script_Object(), type_value);

			MapRandGenTemplateTemplate temp;
			temp.bounds			= world_bb;
			temp.tile_x			= (int)(world_bb.X / tile_width);
			temp.tile_y			= (int)(world_bb.Y / tile_height);
			temp.tile_width		= (int)(world_bb.Width / tile_width);
			temp.tile_height	= (int)(world_bb.Height / tile_height);
			temp.room_width		= (int)(temp.tile_width / template_room_tile_width);
			temp.room_height	= (int)(temp.tile_height / template_room_tile_height);
			temp.type			= (MapRandGenTemplateType::Type)type_value.int_value;
			
			SAFE_DELETE(marker);

			m_template_templates[temp.type] = temp;
		}
	}

	// Load junctions and assign to appropriate room templates.
	for (std::vector<MapFileObjectBlock*>::iterator iter = objects.begin(); iter != objects.end(); iter++)
	{
		MapFileObjectBlock* obj = *iter;
		if (obj->Class_Name == "DC_Junction_Marker")
		{
			ScriptedActor* marker = obj->Create_Instance();

			Rect2D world_bb = marker->Get_World_Bounding_Box();

			MapRandGenJunctionTemplate temp;
			temp.tile_width		= (int)(world_bb.Width / tile_width);
			temp.tile_height	= (int)(world_bb.Height / tile_height);

			for (std::vector<MapRandGenRoomTemplate>::iterator temp_iter = m_room_templates.begin(); temp_iter != m_room_templates.end(); temp_iter++)
			{
				MapRandGenRoomTemplate& t = *temp_iter;
				if (t.bounds.Intersects(world_bb.Center()))
				{
					Vector2 c = world_bb.Center();
					temp.offset_x = (int)((c.X - t.bounds.X) / tile_width);
					temp.offset_y = (int)((c.Y - t.bounds.Y) / tile_height);

					t.junctions.push_back(temp);

					junction_count++;
					break;
				}
			}

			SAFE_DELETE(marker);
		}
	}

	// Load all none-dc objects and add them to room templates they need to be instantiated for.
	for (std::vector<MapFileObjectBlock*>::iterator iter = objects.begin(); iter != objects.end(); iter++)
	{
		MapFileObjectBlock* obj = *iter;
		if (obj->Class_Name.substr(0, 3) != "DC_" || obj->Class_Name == "DC_Room_Marker" || obj->Class_Name == "DC_Template_Marker") // Ignore DC_* markers, we don't want to actually instantiate them in the dungeon rooms.
		{
			ScriptedActor* marker = obj->Create_Instance();
			if (marker == NULL)
			{
				continue;
			}

			Rect2D world_bb = marker->Get_World_Bounding_Box();

			MapRandGenObjectTemplate temp;
			temp.block = obj;

			for (std::vector<MapRandGenRoomTemplate>::iterator temp_iter = m_room_templates.begin(); temp_iter != m_room_templates.end(); temp_iter++)
			{
				MapRandGenRoomTemplate& t = *temp_iter;
				if (t.bounds.Intersects(world_bb.Center()))
				{
					Vector3 pos = marker->Get_Position();

					temp.offset_x = (pos.X - t.bounds.X);
					temp.offset_y = (pos.Y - t.bounds.Y);

					t.objects.push_back(temp);

					object_count++;
					break;
				}
			}

			for (int i = 0; i < MapRandGenTemplateType::COUNT; i++)
			{
				MapRandGenTemplateTemplate& t = m_template_templates[i];
				if (t.bounds.Intersects(world_bb.Center()))
				{
					Vector3 pos = marker->Get_Position();

					temp.offset_x = (pos.X - t.bounds.X);
					temp.offset_y = (pos.Y - t.bounds.Y);

					t.objects.push_back(temp);

					object_count++;
					break;
				}
			}

			SAFE_DELETE(marker);
		}
	}

	DBG_LOG("Loaded template: rooms=%i junctions=%i objects=%i", room_count, junction_count, object_count);
}

std::vector<MapRandGenRoomTemplate*> MapRandGenManager::GetPossibleConnectingRoomTemplates(MapRandGenTreeNode* base, int AbsOffsetFromRootFirst, int AbsOffsetFromRootLast, int AbsOffsetFromSubFirst, int AbsOffsetFromSubLast, bool bOnRootPath)
{
	std::vector<MapRandGenRoomTemplate*> possible;

	MapRandGenRoomType::Type force_type = MapRandGenRoomType::COUNT;

	for (std::vector<MapRandGenRoomTemplate>::iterator temp_iter = m_room_templates.begin(); temp_iter != m_room_templates.end(); temp_iter++)
	{
		MapRandGenRoomTemplate& t = *temp_iter;
		bool bPossible = true;
		bool bShouldForce = false;

		for (std::vector<MapRandGenRoomRule>::iterator root_iter = t.rules.begin(); root_iter != t.rules.end(); root_iter++)
		{
			MapRandGenRoomRule& rule = *root_iter;

			bool bPassed = false;

			// Work out result.
			switch (rule.type)
			{
			// Only a limited number of these rooms can be placed. 
			case MapRandGenRoomRuleType::LimitedCount:
				{
					bPassed = (m_rooms_placed_counts[t.type] < rule.value);
					break;
				}
		
			// Room must be connected to another room of given type.
			case MapRandGenRoomRuleType::ConnectedTo:
				{
					bPassed = (base->room_template != NULL && base->room_template->type == rule.value);
					break;
				}

			// Offset in rooms from the last root room.
			case MapRandGenRoomRuleType::AbsoluteOffsetFromRootLast:
				{
					bPassed = (AbsOffsetFromRootLast == rule.value);
					break;
				}

			// Offset in rooms from the first root room.
			case MapRandGenRoomRuleType::AbsoluteOffsetFromRootFirst:
				{
					bPassed = (AbsOffsetFromRootFirst == rule.value);
					break;
				}

				// Offset in rooms from the last sub room.
			case MapRandGenRoomRuleType::AbsoluteOffsetFromSubLast:
				{
					bPassed = (AbsOffsetFromSubLast == rule.value);
					break;
				}

				// Offset in rooms from the first sub room.
			case MapRandGenRoomRuleType::AbsoluteOffsetFromSubFirst:
				{
					bPassed = (AbsOffsetFromSubFirst == rule.value);
					break;
				}

			// On root path.
			case MapRandGenRoomRuleType::OnRootPath:
				{
					bPassed = bOnRootPath;
					break;
				}

			// Not duplicate of previous room?
			case MapRandGenRoomRuleType::NotDuplicateRoom:
				{
					bPassed = (base->room_template != &t);
					break;
				}

			// Not duplicate of previous room type?
			case MapRandGenRoomRuleType::NotDuplicateRoomType:
				{
					bPassed = (base->room_template == NULL || base->room_template->type != t.type);
					break;
				}

			// Force this room type if we come across it..
			case MapRandGenRoomRuleType::ForceRoomType:
				{
					bPassed = true;
					bShouldForce = true;
					break;
				}
			}

			// Apply operator.
			switch (rule.op)
			{
				case MapRandGenRoomRuleOpType::Equal:
					{
						break;
					}
				case MapRandGenRoomRuleOpType::NotEqual:
					{
						bPassed = !bPassed;
						break;
					}
			}

			// To many instances?
			if (t.max_instances != 0 && bPassed)
			{
				bPassed = (t.instance_count < t.max_instances);
				if (!bPassed)
				{
					DBG_LOG("FAILED TO PLACE ROOM OF TYPE %i, %i >= %i", t.type, t.instance_count, t.max_instances);
				}
			}

			if (!bPassed)
			{
				bPossible = false;
				break;
			}
		}

		if (bPossible)
		{
			possible.push_back(&t);

			if (bShouldForce)
			{
				force_type = t.type;
			}
		}
	}

	// Force a specific room type.
	if (force_type != MapRandGenRoomType::COUNT)
	{
		for (std::vector<MapRandGenRoomTemplate*>::iterator iter = possible.begin(); iter != possible.end(); )
		{
			if ((*iter)->type != force_type)
			{
				iter = possible.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}

	return possible;
}

std::vector<MapRandGenRoomConnection> MapRandGenManager::GetPossibleSpatialConnections(MapRandGenTreeNode* base, std::vector<MapRandGenRoomTemplate*> rooms)
{
	std::vector<MapRandGenRoomConnection> possible;

	// If base is root, then its always possible to connect.
	if (base == &m_root)
	{
		for (std::vector<MapRandGenRoomTemplate*>::iterator map_iter = rooms.begin(); map_iter != rooms.end(); map_iter++)
		{
			MapRandGenRoomTemplate* room = (*map_iter);

			for (std::vector<MapRandGenJunctionTemplate>::iterator junction2_iter = room->junctions.begin(); junction2_iter != room->junctions.end(); junction2_iter++)
			{
				MapRandGenJunctionTemplate* room_junction = &(*junction2_iter);

				MapRandGenRoomConnection connection;
				connection.room_template = room;
				connection.parent_connected_junction = NULL;
				connection.room_connected_junction = room_junction;
				possible.push_back(connection);
			}
		}

		return possible;
	}

	// Collect all possible connections between the maps.
	for (std::vector<MapRandGenJunctionTemplate>::iterator junction_iter = base->room_template->junctions.begin(); junction_iter != base->room_template->junctions.end(); junction_iter++)
	{
		MapRandGenJunctionTemplate* parent_junction = &(*junction_iter);
		
		for (std::vector<MapRandGenRoomTemplate*>::iterator map_iter = rooms.begin(); map_iter != rooms.end(); map_iter++)
		{
			MapRandGenRoomTemplate* room = (*map_iter);

			for (std::vector<MapRandGenJunctionTemplate>::iterator junction2_iter = room->junctions.begin(); junction2_iter != room->junctions.end(); junction2_iter++)
			{
				MapRandGenJunctionTemplate* room_junction = &(*junction2_iter);

				MapRandGenRoomConnection connection;
				connection.room_template = room;
				connection.parent_connected_junction = parent_junction;
				connection.room_connected_junction = room_junction;
				possible.push_back(connection);
			}
		}
	}

	// Prune any connections that cause parent&connected to overlap.
	for (std::vector<MapRandGenRoomConnection>::iterator iter = possible.begin(); iter != possible.end(); )
	{
		MapRandGenRoomConnection& connection = *iter;

		bool bOverlaps = false;

		if (
			(connection.parent_connected_junction->offset_x == 0 && connection.room_connected_junction->offset_x != connection.room_template->tile_width - 1) ||
			(connection.parent_connected_junction->offset_x == connection.room_template->tile_width - 1 && connection.room_connected_junction->offset_x != 0) ||
			(connection.parent_connected_junction->offset_y == 0 && connection.room_connected_junction->offset_y != connection.room_template->tile_height - 1) ||
			(connection.parent_connected_junction->offset_y == connection.room_template->tile_height - 1 && connection.room_connected_junction->offset_y != 0) 	
			)
		{
			bOverlaps = true;
		}

		if (bOverlaps)
		{
			 iter = possible.erase(iter);
		}
		else
		{
			iter++;
		}
	}

	// Prune out any connections that cause spatial overlaps.
	for (std::vector<MapRandGenRoomConnection>::iterator iter = possible.begin(); iter != possible.end(); )
	{
		MapRandGenRoomConnection& connection = *iter;

		bool bCollision = false;

		MapRandGenTreeNode tmp_node;
		tmp_node.parent						= base;
		tmp_node.room_template				= connection.room_template;
		tmp_node.parent_connected_junction	= connection.parent_connected_junction;
		tmp_node.room_connected_junction	= connection.room_connected_junction;

		for (std::vector<MapRandGenTreeNode*>::iterator iter_b = m_nodes.begin(); iter_b != m_nodes.end(); iter_b++)
		{
			if (AreNodesOverlapping(&tmp_node, *iter_b))
			{
				bCollision = true;
				break;
			}
		}

		if (bCollision)
		{
			iter = possible.erase(iter);
		}
		else
		{
			iter++;
		}
	}

	return possible;
}

Rect2D MapRandGenManager::GetNodeBounds(MapRandGenTreeNode* node)
{
	if (node == &m_root)
	{
		return Rect2D(m_origin_x, m_origin_y, 0.0f, 0.0f);
	}
	if (node->parent == &m_root)
	{
		return Rect2D(m_origin_x, m_origin_y, (float)node->room_template->tile_width * m_tile_width, (float)node->room_template->tile_height * m_tile_height);
	}

	if (node->bBoundsCalculated)
	{
		return node->bounds;
	}

	Rect2D parent_bounds = GetNodeBounds(node->parent);

	Vector2 parent_world_joint = Vector2(parent_bounds.X, parent_bounds.Y) + Vector2(node->parent_connected_junction->offset_x * m_tile_width, node->parent_connected_junction->offset_y * m_tile_width);
	Vector2 room_local_joint = Vector2(node->room_connected_junction->offset_x * m_tile_width, node->room_connected_junction->offset_y * m_tile_width);

	Vector2 room_pos = Vector2(parent_world_joint.X - room_local_joint.X, parent_world_joint.Y - room_local_joint.Y);

	// We need to offset depending on what side we are lining up on.
	if (node->parent_connected_junction->offset_x == 0)
	{
		// Left
		room_pos.X -= m_tile_width;
	}
	else if (node->parent_connected_junction->offset_x == node->parent->room_template->tile_width - 1)
	{
		// Right
		room_pos.X += m_tile_width;
	}
	else if (node->parent_connected_junction->offset_y == 0)
	{
		// Up
		room_pos.Y -= m_tile_width;
	}
	else if (node->parent_connected_junction->offset_y == node->parent->room_template->tile_height - 1)
	{
		// Down
		room_pos.Y += m_tile_width;
	}

	node->bBoundsCalculated = true;
	node->bounds = Rect2D(room_pos.X, room_pos.Y, (float)node->room_template->tile_width * m_tile_width, (float)node->room_template->tile_height * m_tile_height);
	return node->bounds;
}

bool MapRandGenManager::AreNodesOverlapping(MapRandGenTreeNode* a, MapRandGenTreeNode* b)
{
	Rect2D a_bounds = GetNodeBounds(a).Inflate(-2.0f, -2.0f);
	Rect2D b_bounds = GetNodeBounds(b).Inflate(-2.0f, -2.0f);

	return a_bounds.Intersects(b_bounds);
}

std::vector<MapRandGenTreeNode*> MapRandGenManager::GeneratePath(MapRandGenTreeNode* base, int length)
{
	MapRandGenTreeNode* original_base = base;

	std::vector<MapRandGenTreeNode*> added;

	for (int i = 0; i < length; i++)
	{
		int AbsOffsetFromRootFirst = 9999;
		int AbsOffsetFromRootLast = 9999;
		int AbsOffsetFromSubFirst = 9999;
		int AbsOffsetFromSubLast = 9999;
		bool bOnRoot = false;

		if (original_base == &m_root)
		{
			bOnRoot = true;
			AbsOffsetFromRootFirst = i;
			AbsOffsetFromRootLast = (length - 1) - i;
		}
		else
		{
			AbsOffsetFromSubFirst = i;
			AbsOffsetFromSubLast = (length - 1) - i;
		}

		// Find all possible room templates that can connect at this level of the tree.
		std::vector<MapRandGenRoomTemplate*> room_templates = GetPossibleConnectingRoomTemplates(base, AbsOffsetFromRootFirst, AbsOffsetFromRootLast, AbsOffsetFromSubFirst, AbsOffsetFromSubLast, bOnRoot);
		if (room_templates.size() <= 0)
		{
		//	DBG_LOG("Failed to find valid room template for path.");
			return added;
		}

		// Create pairs of room+junctions that we can spatially connect at.
		std::vector<MapRandGenRoomConnection> room_connections = GetPossibleSpatialConnections(base, room_templates);
		if (room_connections.size() <= 0)
		{
		//	DBG_LOG("Failed to find valid connections for path.");
			return added;
		}

		// Randomly select one of the possible connections.
		MapRandGenRoomConnection connection = room_connections[m_rand.Next(0, room_connections.size())];

		// Create the room!
		MapRandGenTreeNode* new_node		= m_node_allocator.Alloc();
		DBG_ASSERT_STR(new_node != NULL, "RandGen room allocator ran out of space!");
		DBG_ASSERT_STR(base->children_count < MapRandGenTreeNode::max_children, "RandGen room exceeded maximum children.");

		base->children[base->children_count++] = new_node;

		new_node->parent					= base;
		new_node->room_template				= connection.room_template;
		new_node->parent_connected_junction = connection.parent_connected_junction;
		new_node->room_connected_junction	= connection.room_connected_junction;
		new_node->bPartOfRoot				= (original_base == &m_root);
		
		m_nodes.push_back(new_node);
		m_rooms_placed_counts[new_node->room_template->type]++;
		new_node->room_template->instance_count++;

		DBG_LOG("Added room of type %i", new_node->room_template->type);

		added.push_back(new_node);

		// On to the next one!
		base = new_node;
	}

	return added;
}

void MapRandGenManager::RebaseOrigin()
{
	Rect2D map_bounds = Rect2D(0.0f, 0.0f, 0.0f, 0.0f);

	for (std::vector<MapRandGenTreeNode*>::iterator iter_b = m_nodes.begin(); iter_b != m_nodes.end(); iter_b++)
	{		
		MapRandGenTreeNode* node = *iter_b;
		map_bounds = map_bounds.Union(GetNodeBounds(node)); 
	}

	m_origin_x = -map_bounds.X;
	m_origin_y = -map_bounds.Y;

	Rect2D final_map_bounds = Rect2D(0.0f, 0.0f, 0.0f, 0.0f);
	
	for (std::vector<MapRandGenTreeNode*>::iterator iter_b = m_nodes.begin(); iter_b != m_nodes.end(); iter_b++)
	{		
		MapRandGenTreeNode* node = *iter_b;
		node->bBoundsCalculated = false;
	}

	for (std::vector<MapRandGenTreeNode*>::iterator iter_b = m_nodes.begin(); iter_b != m_nodes.end(); iter_b++)
	{		
		MapRandGenTreeNode* node = *iter_b;
		final_map_bounds = final_map_bounds.Union(GetNodeBounds(node));
	}

	m_map_bounds = final_map_bounds;
}

void MapRandGenManager::DumpDebugPreview(const char* map_file)
{
	Pixelmap* pix = new Pixelmap((int)m_map_bounds.Width, (int)m_map_bounds.Height, PixelmapFormat::R8G8B8A8);
	
	int node_index = 0;
	for (std::vector<MapRandGenTreeNode*>::iterator iter_b = m_nodes.begin(); iter_b != m_nodes.end(); iter_b++, node_index++)
	{		
		MapRandGenTreeNode* node = *iter_b;
		Rect2D bounds = GetNodeBounds(node);

		for (int x = (int)bounds.X; x < (int)bounds.X + (int)bounds.Width; x++)
		{
			for (int y = (int)bounds.Y; y <(int) bounds.Y + (int)bounds.Height; y++)
			{
				if (node->bPartOfRoot)
				{
					pix->Set_Pixel(x, y, Color(256, 128 + ((128 * node_index) % 128), 128 + ((64 * node_index) % 128), 255));
				}
				else
				{
					pix->Set_Pixel(x, y, Color((16 * node_index) % 64, (128 * node_index) % 64, (64 * node_index) % 64, 255));
				}
			}
		}

		Rect2D junction_bounds = Rect2D(
			bounds.X + (node->room_connected_junction->offset_x * m_tile_width) - 8.0f,
			bounds.Y + (node->room_connected_junction->offset_y * m_tile_height) - 8.0f,
			16.0f,
			16.0f
		);

		for (int x = (int)junction_bounds.X; x < (int)junction_bounds.X + (int)junction_bounds.Width; x++)
		{
			for (int y = (int)junction_bounds.Y; y <(int) junction_bounds.Y + (int)junction_bounds.Height; y++)
			{
				pix->Set_Pixel(x, y, Color::Red);
			}
		}
	}

	PixelmapFactory::Save(map_file, pix);
}

void MapRandGenManager::PasteObjects(Map* map, MapRandGenTreeNode* node, int to_x, int to_y, std::vector<MapRandGenObjectTemplate> objects)
{
	Scene* scene = GameEngine::Get()->Get_Scene();
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	for (std::vector<MapRandGenObjectTemplate>::iterator iter = objects.begin(); iter != objects.end(); iter++)
	{
		MapRandGenObjectTemplate& obj = *iter;
	
		ScriptedActor* scripted = obj.block->Create_Instance();
		if (scripted != NULL)
		{
		//	DBG_LOG("Creating object '%s'.", obj.block->Class_Name.c_str());

			Vector3 position = Vector3(to_x + obj.offset_x, to_y + obj.offset_y, 0.0f);

			scripted->Set_Position(position);

			if (!scripted->Get_Script_Object().Get()->Get_Symbol()->symbol->class_data->is_replicated)
				scripted->Set_Map_ID(map->Create_Map_ID());

			Vector3 center = scripted->Get_Collision_Center();

			// Prefix tags with room number.
			if (scripted->Get_Tag() != "")
			{
				scripted->Set_Tag(StringHelper::Format("room_0x%08x_%s", node, scripted->Get_Tag().c_str()));
			}			
			if (scripted->Get_Link() != "")
			{
				scripted->Set_Link(StringHelper::Format("room_0x%08x_%s", node, scripted->Get_Link().c_str()));
			}

			// Go through vars and replace all links.
			// TODO: Fix so it deals with inheritance.
			CVMLinkedSymbol* sym = scripted->Get_Script_Symbol();
			for (int i = 0; i < sym->symbol->class_data->field_count; i++)
			{
				CVMLinkedSymbol* field = vm->Get_Symbol_Table_Entry(sym->symbol->class_data->fields_indexes[i]);
				if (field->symbol->Get_Meta_Data<int>("Is_Link", 0) == 1)
				{
					CVMValue val;
					vm->Get_Field(field, scripted->Get_Script_Object(), val);
					if (val.string_value != "")
					{
						val.string_value = StringHelper::Format("room_0x%08x_%s", node, val.string_value.C_Str()).c_str();
						vm->Set_Field(field, scripted->Get_Script_Object(), val);
					}
				}
			}

			scene->Add_Actor(scripted);
			scene->Add_Tickable(scripted);

			node->objects.push_back(scripted);
		}
	}
}

void MapRandGenManager::PasteTilemap(Map* map, std::vector<MapFileLayerBlock*>& layers, int to_x, int to_y, int from_x, int from_y, int width, int height, int only_layer)
{
	int tilemap_width = map->Get_Width();
	int tilemap_height = map->Get_Height();

	for (int z = 0; z < (int)layers.size(); z++)
	{
		MapFileLayerBlock* layer = layers[z];

		if (z < only_layer && only_layer != -1)
		{
			continue;
		}

		for (int x = to_x; x < to_x + width; x++)
		{
			for (int y = to_y; y < to_y + height; y++)
			{
				int template_x = from_x + (x - to_x);
				int template_y = from_y + (y - to_y);

				if (x >= 0 && x < tilemap_width && y >= 0 && y < tilemap_height)
				{
					*map->Get_Tile(x, y, z) = layer->Tiles[(template_y * layer->Width) + template_x];
				}
			}
		}
	}
}

void MapRandGenManager::Generate(Map* map, MapFile* map_template, int seed)
{
	double start_time = Platform::Get()->Get_Ticks();

	Scene* scene = GameEngine::Get()->Get_Scene();

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	CVMLinkedSymbol* dungeon_blackout_symbol = vm->Find_Class("Dungeon_Blackout");
	CVMLinkedSymbol* door_symbol = vm->Find_Class("Dungeon_Door");
	CVMLinkedSymbol* locked_door_symbol = vm->Find_Variable(door_symbol, "Is_Locked");
	CVMLinkedSymbol* open_door_symbol = vm->Find_Variable(door_symbol, "Is_Open");
	CVMLinkedSymbol* open_animation_symbol = vm->Find_Variable(door_symbol, "Open_Animation");
	CVMLinkedSymbol* partner_door_symbol = vm->Find_Variable(door_symbol, "Partner_Door");

	// Step 1: Load template information from provided map.
	DBG_LOG("Loading random map template ...");
	LoadTemplate(map_template);

	// Grab template info.
	std::vector<MapFileObjectBlock*> template_objects = map_template->Get_Blocks<MapFileObjectBlock>();
	std::vector<MapFileLayerBlock*>  template_layers = map_template->Get_Blocks<MapFileLayerBlock>();

	AtlasFrame* frame = map_template->Get_Header()->Tileset->Get()->Get_Frame_By_Index(0);
	int tile_width = (int)frame->Rect.Width;
	int tile_height = (int)frame->Rect.Height;

	Random seed_rand(seed);

	for (int attempts = 0; attempts < sub_path_place_reattempts; attempts++, seed = seed_rand.Next())
	{
		DBG_LOG("Generating random map using seed 0x%08x, attempt %i ...", seed, attempts);

		// Kill everything in scene.
		Game::Get()->Get_Game_Scene()->Unload();
		vm->Get_GC()->Collect();

		// Send the random generate.
		m_rand = Random(seed);

		// Calculate some global random values.
		int root_path_length = m_rand.Next(min_root_path_length, max_root_path_length);
		int sub_path_count = m_rand.Next(min_sub_path_count, max_sub_path_count);

		// Reset everything ready for generation.
		m_root.room_template = NULL;
		m_root.parent_connected_junction = NULL;
		m_root.room_connected_junction = NULL;
		m_root.parent = NULL;
		m_root.children_count = 0;

		m_origin_x = 0.0f;
		m_origin_y = 0.0f;

		m_nodes.clear();
		m_node_allocator.Free();

		for (int i = 0; i < MapRandGenRoomType::COUNT; i++)
		{
			m_rooms_placed_counts[i] = 0;
		}

		// ----------------------------------------------------------------------------------------------------
		// Step 2: Generate the root path for the map.
		// ----------------------------------------------------------------------------------------------------
		std::vector<MapRandGenTreeNode*> root_path = GeneratePath(&m_root, root_path_length);
		if ((int)root_path.size() < root_path_length)
		{
			continue;
		}

		// ----------------------------------------------------------------------------------------------------
		// Step 3: Generate branches off the map.
		// ----------------------------------------------------------------------------------------------------
		int current_sub_path_count = 0;
		int sub_path_attempts = sub_path_place_reattempts;

		std::vector<std::vector<MapRandGenTreeNode*> > sub_paths;

		while (current_sub_path_count < sub_path_count && sub_path_attempts > 0)
		{
			int sub_path_length = m_rand.Next(min_sub_path_length, max_sub_path_length);

			MapRandGenTreeNode* sub_root = NULL;
			while (sub_root == NULL || !sub_root->bPartOfRoot)
			{
				int idx = m_rand.Next(0, (int)m_nodes.size() - 1); 
				DBG_LOG("Selecting %i / %u", idx, m_nodes.size());
				sub_root = m_nodes.at(idx);
			}

			std::vector<MapRandGenTreeNode*> sub_path = GeneratePath(sub_root, sub_path_length);
			if (sub_path.size() > 1)
			{
				current_sub_path_count++;
				sub_paths.push_back(sub_path);
			}

			sub_path_attempts--;
		}

		// ----------------------------------------------------------------------------------------------------
		// Step 4: Rebase all rooms so origin is 0,0.
		// ----------------------------------------------------------------------------------------------------
		RebaseOrigin();

		// ----------------------------------------------------------------------------------------------------
		// Step 5: Copy all the template rooms onto the actual tilemap.
		// ----------------------------------------------------------------------------------------------------
		map->Resize((int)(m_map_bounds.Width / m_tile_width), (int)(m_map_bounds.Height / m_tile_height), template_layers.size());
		for (std::vector<MapRandGenTreeNode*>::iterator iter_b = m_nodes.begin(); iter_b != m_nodes.end(); iter_b++)
		{		
			MapRandGenTreeNode* node = *iter_b;
			Rect2D bounds = GetNodeBounds(node);

			int tile_x = (int)(bounds.X / tile_width);
			int tile_y = (int)(bounds.Y / tile_height);
			int tile_w = (int)(bounds.Width / tile_width);
			int tile_h = (int)(bounds.Height / tile_height);

			for (int z = 0; z < (int)template_layers.size(); z++)
			{
				MapFileLayerBlock* layer = template_layers[z];

				for (int x = tile_x; x < tile_x + tile_w; x++)
				{
					for (int y = tile_y; y < tile_y + tile_h; y++)
					{
						int template_x = node->room_template->tile_x + (x - tile_x);
						int template_y = node->room_template->tile_y + (y - tile_y);

						*map->Get_Tile(x, y, z) = layer->Tiles[(template_y * layer->Width) + template_x];
					}
				}
			}
		}

		// ----------------------------------------------------------------------------------------------------
		// Step 6: Set bounds.
		// ----------------------------------------------------------------------------------------------------
		scene->Set_Boundries(Rect2D(0.0f, 0.0f, m_map_bounds.Width, m_map_bounds.Height));

		// ----------------------------------------------------------------------------------------------------
		// Step 7: Fill in collision & tiles for junctions not being used.
		// ----------------------------------------------------------------------------------------------------
		for (std::vector<MapRandGenTreeNode*>::iterator iter_b = m_nodes.begin(); iter_b != m_nodes.end(); iter_b++)
		{		
			MapRandGenTreeNode* node = *iter_b;

			Rect2D bounds = GetNodeBounds(node);

			int tile_x = (int)(bounds.X / tile_width);
			int tile_y = (int)(bounds.Y / tile_height);
			int tile_w = (int)(bounds.Width / tile_width);
			int tile_h = (int)(bounds.Height / tile_height);

			for (std::vector<MapRandGenJunctionTemplate>::iterator iter = node->room_template->junctions.begin(); iter != node->room_template->junctions.end(); iter++)
			{
				MapRandGenJunctionTemplate* junction = &(*iter);

				bool bUsed = false;
				for (int i = 0; i < node->children_count; i++)
				{
					if (node->children[i]->parent_connected_junction == junction)
					{
						bUsed = true;
						break;
					}
				}

				if (node->room_connected_junction == junction)
				{
					bUsed = true;
				}
				if (node->parent != NULL && node->parent->room_connected_junction == junction)
				{
					bUsed = true;
				}

				// Not used, fill in the collision.
				if (!bUsed)
				{
					bool finished = false;

					// Top
					if (junction->offset_y == 0)
					{
						// Move down until we find the walls collision.
						for (int y = 1; y < 16 && !finished; y++)
						{
							for (int t = 2; t < 4 && !finished; t++)
							{
								MapTile* tile = map->Get_Tile(tile_x + junction->offset_x - t, tile_y + junction->offset_y + y, collision_layer);
								if (tile->CollisionFrame > 0 && tile->Collision != MapTileCollisionType::None)
								{
									PasteTilemap
									(
										map,
										template_layers,
										(tile_x + junction->offset_x) - (m_template_templates[MapRandGenTemplateType::Unused_Door_Top].tile_width / 2),
										(tile_y + junction->offset_y + y) - (m_template_templates[MapRandGenTemplateType::Unused_Door_Top].tile_height / 2),
										m_template_templates[MapRandGenTemplateType::Unused_Door_Top].tile_x,
										m_template_templates[MapRandGenTemplateType::Unused_Door_Top].tile_y,
										m_template_templates[MapRandGenTemplateType::Unused_Door_Top].tile_width,
										m_template_templates[MapRandGenTemplateType::Unused_Door_Top].tile_height,
										collision_layer
									);
									PasteObjects
									(
										map,
										node,
										((tile_x + junction->offset_x) - (m_template_templates[MapRandGenTemplateType::Unused_Door_Top].tile_width / 2)) * tile_width,
										((tile_y + junction->offset_y + y) - (m_template_templates[MapRandGenTemplateType::Unused_Door_Top].tile_height / 2)) * tile_height,
										m_template_templates[MapRandGenTemplateType::Unused_Door_Top].objects
									);
									finished = true;
								}
							}
						}
					}
					// Bottom
					else if (junction->offset_y == node->room_template->tile_height - 1)
					{
						// Move down until we find the walls collision.
						for (int y = 1; y < 16 && !finished; y++)
						{
							for (int t = 2; t < 4 && !finished; t++)
							{
								MapTile* tile = map->Get_Tile(tile_x + junction->offset_x - t, tile_y + junction->offset_y - y, collision_layer);
								if (tile->CollisionFrame > 0 && tile->Collision != MapTileCollisionType::None)
								{
									PasteTilemap
									(
										map,
										template_layers,
										(tile_x + junction->offset_x) - (m_template_templates[MapRandGenTemplateType::Unused_Door_Bottom].tile_width / 2),
										(tile_y + junction->offset_y - y) - (m_template_templates[MapRandGenTemplateType::Unused_Door_Bottom].tile_height / 2),
										m_template_templates[MapRandGenTemplateType::Unused_Door_Bottom].tile_x,
										m_template_templates[MapRandGenTemplateType::Unused_Door_Bottom].tile_y,
										m_template_templates[MapRandGenTemplateType::Unused_Door_Bottom].tile_width,
										m_template_templates[MapRandGenTemplateType::Unused_Door_Bottom].tile_height,
										collision_layer
										);
									PasteObjects
										(
										map,
										node,
										((tile_x + junction->offset_x) - (m_template_templates[MapRandGenTemplateType::Unused_Door_Bottom].tile_width / 2)) * tile_width,
										((tile_y + junction->offset_y - y) - (m_template_templates[MapRandGenTemplateType::Unused_Door_Bottom].tile_height / 2)) * tile_height,
										m_template_templates[MapRandGenTemplateType::Unused_Door_Bottom].objects
										);
									finished = true;
								}
							}
						}
					}
					// Left
					else if (junction->offset_x == 0)
					{
						// Move down until we find the walls collision.
						for (int x = 1; x < 16 && !finished; x++)
						{
							for (int t = 2; t < 4 && !finished; t++)
							{
								MapTile* tile = map->Get_Tile(tile_x + junction->offset_x + x, tile_y + junction->offset_y - t, collision_layer);
								if (tile->CollisionFrame > 0 && tile->Collision != MapTileCollisionType::None)
								{
									PasteTilemap
										(
										map,
										template_layers,
										(tile_x + junction->offset_x + x) - (m_template_templates[MapRandGenTemplateType::Unused_Door_Left].tile_width / 2),
										(tile_y + junction->offset_y) - (m_template_templates[MapRandGenTemplateType::Unused_Door_Left].tile_height / 2),
										m_template_templates[MapRandGenTemplateType::Unused_Door_Left].tile_x,
										m_template_templates[MapRandGenTemplateType::Unused_Door_Left].tile_y,
										m_template_templates[MapRandGenTemplateType::Unused_Door_Left].tile_width,
										m_template_templates[MapRandGenTemplateType::Unused_Door_Left].tile_height,
										collision_layer
										);
									PasteObjects
										(
										map,
										node,
										((tile_x + junction->offset_x + x) - (m_template_templates[MapRandGenTemplateType::Unused_Door_Left].tile_width / 2)) * tile_width,
										((tile_y + junction->offset_y) - (m_template_templates[MapRandGenTemplateType::Unused_Door_Left].tile_height / 2)) * tile_height,
										m_template_templates[MapRandGenTemplateType::Unused_Door_Left].objects
										);
									finished = true;
								}
							}
						}
					}
					// Right
					else if (junction->offset_x == node->room_template->tile_width - 1)
					{
						// Move down until we find the walls collision.
						for (int x = 1; x < 16 && !finished; x++)
						{
							for (int t = 2; t < 4 && !finished; t++)
							{
								MapTile* tile = map->Get_Tile(tile_x + junction->offset_x - x, tile_y + junction->offset_y - t, collision_layer);
								if (tile->CollisionFrame > 0 && tile->Collision != MapTileCollisionType::None)
								{
									PasteTilemap
										(
										map,
										template_layers,
										(tile_x + junction->offset_x - x) - (m_template_templates[MapRandGenTemplateType::Unused_Door_Right].tile_width / 2),
										(tile_y + junction->offset_y) - (m_template_templates[MapRandGenTemplateType::Unused_Door_Right].tile_height / 2),
										m_template_templates[MapRandGenTemplateType::Unused_Door_Right].tile_x,
										m_template_templates[MapRandGenTemplateType::Unused_Door_Right].tile_y,
										m_template_templates[MapRandGenTemplateType::Unused_Door_Right].tile_width,
										m_template_templates[MapRandGenTemplateType::Unused_Door_Right].tile_height,
										collision_layer
										);
									PasteObjects
										(
										map,
										node,
										((tile_x + junction->offset_x - x) - (m_template_templates[MapRandGenTemplateType::Unused_Door_Right].tile_width / 2)) * tile_width,
										((tile_y + junction->offset_y) - (m_template_templates[MapRandGenTemplateType::Unused_Door_Right].tile_height / 2)) * tile_height,
										m_template_templates[MapRandGenTemplateType::Unused_Door_Right].objects
										);
									finished = true;
								}
							}
						}
					}
				}
			}
		}

		// ----------------------------------------------------------------------------------------------------
		// Step 10: Add key/door puzzles.
		// ----------------------------------------------------------------------------------------------------
		if (sub_paths.size() > 0)
		{
			int locked_door_count = sub_paths.size() == 1 ? 1 : m_rand.Next(1, sub_paths.size());
			std::vector<int> used_sub_paths;
			for (int i = 0; i < locked_door_count; i++)
			{
				// Choose sub-path to use.
				int sub_path = m_rand.Next(0, sub_paths.size() - 1);
				while (std::find(used_sub_paths.begin(), used_sub_paths.end(), sub_path) != used_sub_paths.end())
					sub_path = m_rand.Next(0, sub_paths.size() - 1);

				used_sub_paths.push_back(sub_path);

				// Grab the start/end nodes of the sub-path.
				std::vector<MapRandGenTreeNode*>& path = sub_paths[sub_path];
				MapRandGenTreeNode* lock_room = path[0]->parent;
				MapRandGenTreeNode* key_room = path[path.size() - 1];

				// We only want to put keys in treasure rooms (as they have spawn locations for them).
				if (key_room->room_template->type != MapRandGenRoomType::Treasure)
				{
					continue;
				}

				// The junction we want to lock is the one that goes next in the root path.
				MapRandGenTreeNode* next_root_room = root_path[(std::find(root_path.begin(), root_path.end(), lock_room) - root_path.begin()) + 1];
				MapRandGenJunctionTemplate* junction = next_root_room->parent_connected_junction;

				// Place key in the last room of the sub-path.
				key_room->bSpawnKey = true;

				// Find the door for the given junction and lock it.
				lock_room->locked_junctions.push_back(junction);
			}
		}

		// ----------------------------------------------------------------------------------------------------
		// Step 8: Populate rooms with randomised actors from the templates.
		// ----------------------------------------------------------------------------------------------------
		std::vector<ScriptedActor*> minimap_instances;

		for (std::vector<MapRandGenTreeNode*>::iterator iter_b = m_nodes.begin(); iter_b != m_nodes.end(); iter_b++)
		{		
			MapRandGenTreeNode* node = *iter_b;
			Rect2D bounds = GetNodeBounds(node);

			for (std::vector<MapRandGenObjectTemplate>::iterator iter = node->room_template->objects.begin(); iter != node->room_template->objects.end(); iter++)
			{
				MapRandGenObjectTemplate& obj = *iter;

				// Ignore keys in non-key rooms.
				if (obj.block->Class_Name == "Key_Pickup" && !node->bSpawnKey)
				{
					continue;
				}

				// Can return NULL if server constructed instance.
				ScriptedActor* scripted = obj.block->Create_Instance();
				if (scripted != NULL)
				{
				//	DBG_LOG("Creating object '%s'.", obj.block->Class_Name.c_str());

					Vector3 position = Vector3(bounds.X + obj.offset_x, bounds.Y + obj.offset_y, 0.0f);

					scripted->Set_Position(position);

					if (!scripted->Get_Script_Object().Get()->Get_Symbol()->symbol->class_data->is_replicated)
						scripted->Set_Map_ID(map->Create_Map_ID());

					Vector3 center = scripted->Get_Collision_Center();

					// Make sure there isn't collision at our center, if there is then delete - it will probably
					// be because the room creation filled in the area with collision.
					if (obj.block->Class_Name == "Dungeon_Door")
					{
						MapTile* tile = map->Get_Tile((int)(center.X / tile_width), (int)(center.Y / tile_height), 2);
						if (tile->Collision == MapTileCollisionType::Solid)
						{
							SAFE_DELETE(scripted);
							continue;
						}
					}

					// Store minimap instances for later.
					if (obj.block->Class_Name == "Map_Pickup")
					{
						minimap_instances.push_back(scripted);
					}

					// Prefix tags with room number.
					if (scripted->Get_Tag() != "")
					{
						scripted->Set_Tag(StringHelper::Format("room_0x%08x_%s", node, scripted->Get_Tag().c_str()));
					}			
					if (scripted->Get_Link() != "")
					{
						scripted->Set_Link(StringHelper::Format("room_0x%08x_%s", node, scripted->Get_Link().c_str()));
					}

					// Go through vars and replace all links.
					// TODO: Fix so it deals with inheritance.
					CVMLinkedSymbol* sym = scripted->Get_Script_Symbol();
					for (int i = 0; i < sym->symbol->class_data->field_count; i++)
					{
						CVMLinkedSymbol* field = vm->Get_Symbol_Table_Entry(sym->symbol->class_data->fields_indexes[i]);
						if (field->symbol->Get_Meta_Data<int>("Is_Link", 0) == 1)
						{
							CVMValue val;
							vm->Get_Field(field, scripted->Get_Script_Object(), val);
							if (val.string_value != "")
							{
								val.string_value = StringHelper::Format("room_0x%08x_%s", node, val.string_value.C_Str()).c_str();
								vm->Set_Field(field, scripted->Get_Script_Object(), val);
							}
						}
					}

					scene->Add_Actor(scripted);
					scene->Add_Tickable(scripted);

					node->objects.push_back(scripted);
				}
			}
		}

		// ----------------------------------------------------------------------------------------------------
		// Step 9: Delete all but one minimap instance.
		// ----------------------------------------------------------------------------------------------------
		if (minimap_instances.size() > 0)
		{
			int index = m_rand.Next() % minimap_instances.size();

			for (int i = 0; i < (int)minimap_instances.size(); i++)
			{
				if (i != index)
				{
					SAFE_DELETE(minimap_instances[i]);
				}
			}			

			minimap_instances.clear();
		}

		// ----------------------------------------------------------------------------------------------------
		// Step 9: Lock rooms.
		// ----------------------------------------------------------------------------------------------------
		for (std::vector<MapRandGenTreeNode*>::iterator iter_b = m_nodes.begin(); iter_b != m_nodes.end(); iter_b++)
		{		
			MapRandGenTreeNode* node = *iter_b;

			for (std::vector<MapRandGenJunctionTemplate*>::iterator iter = node->locked_junctions.begin(); iter != node->locked_junctions.end(); iter++)
			{
				MapRandGenJunctionTemplate* junction = *iter;

				Rect2D junction_bbox = Rect2D(
					(float)node->bounds.X + (junction->offset_x * tile_width), 
					(float)node->bounds.Y + (junction->offset_y * tile_height),
					(float)junction->tile_width * tile_width, 
					(float)junction->tile_height * tile_height);

				Vector2 junction_center = junction_bbox.Center();

				ScriptedActor* closest = NULL;
				float closest_distance = 0.0f;

				// Top / Bottom
				if (junction->offset_y == 0 ||
					junction->offset_y == node->room_template->tile_height - 1)
				{
					for (std::vector<ScriptedActor*>::iterator iter_c = node->objects.begin(); iter_c != node->objects.end(); iter_c++)
					{
						ScriptedActor* actor = *iter_c;
						if (actor->Get_Script_Symbol() != door_symbol)
						{
							continue;
						}

						Rect2D actor_bbox = actor->Get_World_Bounding_Box();
						if (junction_center.X >= actor_bbox.X && junction_center.X < actor_bbox.X + actor_bbox.Width)
						{
							float d = fabs((actor_bbox.Center() - junction_center).Y);
							if (closest == NULL || d < closest_distance)
							{
								closest = actor;
								closest_distance = d;
							}
						}
					}
				}
				// Left / Right
				else if (junction->offset_x == 0 ||
						 junction->offset_x == node->room_template->tile_width - 1)
				{
					for (std::vector<ScriptedActor*>::iterator iter_c = node->objects.begin(); iter_c != node->objects.end(); iter_c++)
					{
						ScriptedActor* actor = *iter_c;
						if (actor->Get_Script_Symbol() != door_symbol)
						{
							continue;
						}

						Rect2D actor_bbox = actor->Get_World_Bounding_Box();
						if (junction_center.Y >= actor_bbox.Y && junction_center.Y < actor_bbox.Y + actor_bbox.Height)
						{
							float d = fabs((actor_bbox.Center() - junction_center).X);
							if (closest == NULL || d < closest_distance)
							{
								closest = actor;
								closest_distance = d;
							}
						}
					}
				}

				if (closest != NULL)
				{
					// Is_Locked = true
					CVMValue value;
					value.int_value = 1;
					vm->Set_Field(locked_door_symbol, closest->Get_Script_Object(), value);

					// Is_Open = false;
					value.int_value = 0;
					vm->Set_Field(open_door_symbol, closest->Get_Script_Object(), value);

					vm->Get_Field(open_animation_symbol, closest->Get_Script_Object(), value);
					if (!value.string_value.Contains("locked"))
					{
						value.string_value = StringHelper::Replace(value.string_value.C_Str(), "_open", "_locked_open").c_str();

						// Animation = xxx
						vm->Set_Field(open_animation_symbol, closest->Get_Script_Object(), value);
					}
				}
			}
		}

		// ----------------------------------------------------------------------------------------------------
		// Step 10: Update all doors so they are linked to their partner doors.
		//			This is done by just traversing the entire tree and setting each door to be a parent of
		//			the door before it.
		// ----------------------------------------------------------------------------------------------------
		std::vector<Actor*> doors = GameEngine::Get()->Get_Scene()->Get_Actors_Of_Class(door_symbol);

		for (std::vector<MapRandGenTreeNode*>::iterator iter_b = m_nodes.begin(); iter_b != m_nodes.end(); iter_b++)
		{
			MapRandGenTreeNode* room = *iter_b;
			Rect2D room_bounds = room->bounds;

			for (std::vector<ScriptedActor*>::iterator iter_c = room->objects.begin(); iter_c != room->objects.end(); iter_c++)
			{
				ScriptedActor* door = *iter_c;
				if (door->Get_Script_Symbol() != door_symbol)
				{
					continue;
				}

				Vector2 search_vector = Vector2(0.0f, 0.0f);

				Rect2D door_bbox = door->Get_Bounding_Box();

				const float EPSILON = 16.0f;

				// Already figured out partner.
				CVMValue partner_value;
				vm->Get_Field(partner_door_symbol, door->Get_Script_Object(), partner_value);
				if (partner_value.string_value != "")
				{
					continue;
				}

				CVMValue anim_value;
				vm->Get_Field(open_animation_symbol, door->Get_Script_Object(), anim_value);

				// Top
				if (anim_value.string_value.Contains("_up"))
				{
					search_vector = Vector2(0.0f, -1.0f);
				}
				// Bottom
				else if (anim_value.string_value.Contains("_down"))
				{
					search_vector = Vector2(0.0f, 1.0f);
				}
				// Left
				else if (anim_value.string_value.Contains("_left"))
				{
					search_vector = Vector2(-1.0f, 0.0f);
				}
				// Right
				else if (anim_value.string_value.Contains("_right"))
				{
					search_vector = Vector2(1.0f, 0.0f);
				}
				// Erm .... Ignore this door but dump a log.
				else
				{
					DBG_LOG("[WARNING] Failed to find door's partner, animation appears invalid? '%s'", anim_value.string_value.C_Str());
					continue;
				}

				// Step down search vector looking for door.
				bool bFound = false;
				ScriptedActor* partner_door = NULL;

				float step_distance = 16.0f;
				for (int step = 0; step < 16 && !bFound; step++)
				{
					Vector2 check_pos = door->Get_World_Bounding_Box().Center() + (Vector2(search_vector.X * step, search_vector.Y * step) * Vector2(step_distance, step_distance));
					Rect2D check_rect = Rect2D(check_pos.X - (step_distance * 0.5f), check_pos.Y - (step_distance * 0.5f), step_distance, step_distance);

					for (std::vector<Actor*>::iterator door_iter = doors.begin(); door_iter != doors.end(); door_iter++)
					{
						ScriptedActor* other_door = dynamic_cast<ScriptedActor*>(*door_iter);
						if (other_door != door && other_door->Get_World_Bounding_Box().Intersects(check_rect))
						{
							bFound = true;
							partner_door = other_door;
							break;
						}
					}
				}

				if (partner_door)
				{
					partner_value.string_value = door->Get_Tag().c_str();
					vm->Set_Field(partner_door_symbol, partner_door->Get_Script_Object(), partner_value);

					partner_value.string_value = partner_door->Get_Tag().c_str();
					vm->Set_Field(partner_door_symbol, door->Get_Script_Object(), partner_value);
				}
				else
				{
					DBG_LOG("[WARNING] Failed to find door's partner, could not be found along search path?", anim_value.string_value.C_Str());
				}
			}
		}

		// ----------------------------------------------------------------------------------------------------
		// Step 9: Add fade in/out planes.
		// ----------------------------------------------------------------------------------------------------
		for (std::vector<MapRandGenTreeNode*>::iterator iter_b = m_nodes.begin(); iter_b != m_nodes.end(); iter_b++)
		{		
			MapRandGenTreeNode* node = *iter_b;
			Rect2D bounds = GetNodeBounds(node);

			ScriptedActor* actor = Game::Get()->Get_Game_Scene()->Spawn(dungeon_blackout_symbol, NULL);
			actor->Set_Layer(4);
			actor->Set_Position(Vector3(bounds.X, bounds.Y, 0.0f));
			actor->Set_Bounding_Box(Rect2D(0.0f, 0.0f, bounds.Width, bounds.Height));
		}

		// We're have a valid map! Break out.
		break;
	}

	// Dump logging information.
	double elapsed = Platform::Get()->Get_Ticks() - start_time;
	DBG_LOG("Gernerated random map in %.2f ms.", elapsed);
}