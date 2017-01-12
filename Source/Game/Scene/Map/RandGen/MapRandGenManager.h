// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_MAP_MAPRANDGENMANAGER_
#define _ENGINE_MAP_MAPRANDGENMANAGER_

#include "Generic/Types/HashTable.h"

#include "Engine/Tasks/Task.h"

#include "Engine/Scene/Map/Map.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Game/Scene/Map/Blocks/MapFileLayerBlock.h"
#include "Game/Scene/Map/Blocks/MapFileObjectBlock.h"

#include "Generic/Math/Random.h"

#include "Generic/Types/StackAllocator.h"

#include <vector>

struct MapRandGenJunctionTemplate
{
	// Offset (in tiles) within the room template that this junction is located at.
	int offset_x;
	int offset_y;

	// Size of this junction in tiles.
	int tile_width;
	int tile_height;
};

struct MapRandGenObjectTemplate
{
	// Offset (in pixels) within the room template that this object is located at.
	float offset_x;
	float offset_y;

	// Map block used to construct this object.
	MapFileObjectBlock* block;
};

struct MapRandGenRoomType
{
	enum Type
	{
		Arena			= 0,				// Doors lock, enemies spawn, no continue until all dead.
		Boss			= 1,				// Doors lock, epic boss spawns, no continue until dead. Next room is usually end.
		Corridoor		= 2,				// Just a bit of walking, maybe a trap or two.
		Start			= 3,				// Starting room.
		End				= 4,				// Ending room.
		Resupply		= 5,				// Weapon/ammo resupply room.
		Treasure		= 6,				// Random treasure room.
		Boss_Treasure	= 7,				// Big boss treasure room.

		COUNT
	};

	static const char* Mnemonics[COUNT];
};

struct MapRandGenTemplateType
{
	enum Type
	{
		Unused_Door_Top			= 0,
		Unused_Door_Bottom		= 1,
		Unused_Door_Left		= 2,
		Unused_Door_Right		= 3,

		COUNT
	};
};

struct MapRandGenTemplateTemplate
{
	// Type of template.
	MapRandGenTemplateType::Type type;

	// World bounds in template map.
	Rect2D bounds;

	// Dimensions in tiles.
	int tile_width;
	int tile_height;
	int tile_x;
	int tile_y;

	// Dimensions in room template sizes. (tile_width / template_room_tile_width)
	int room_width;
	int room_height;

	// Objects to be placed in the room.
	std::vector<MapRandGenObjectTemplate> objects;
};

struct MapRandGenRoomRuleOpType
{
	enum Type
	{
		Equal,
		NotEqual,

		COUNT
	};
};

struct MapRandGenRoomRuleType
{
	enum Type
	{
		LimitedCount,					// Only a limited number of these rooms can be placed. 
		ConnectedTo,					// Room must be connected to another room of given type.
	
		AbsoluteOffsetFromRootLast,		// Offset in rooms from the last root room.
		AbsoluteOffsetFromRootFirst,	// Offset in rooms from the first root room.

		AbsoluteOffsetFromSubLast,		// Offset in rooms from the last sub-path room.
		AbsoluteOffsetFromSubFirst,		// Offset in rooms from the first sub-path room.

		OnRootPath,						// On root path?

		NotDuplicateRoom,				// Not duplicate of previous room?
		NotDuplicateRoomType,			// Not duplicate of previous room type?

		ForceRoomType,					// Force this room type if we come across it..

		COUNT
	};
};

struct MapRandGenRoomRule
{
	// Type of rule.
	MapRandGenRoomRuleType::Type type;

	// Operation.
	MapRandGenRoomRuleOpType::Type op;

	// Values dependent on type.
	int value;

	MapRandGenRoomRule(MapRandGenRoomRuleOpType::Type inOp, MapRandGenRoomRuleType::Type inType, int inValue)
		: type(inType)
		, op(inOp)
		, value(inValue)
	{
	}
};

struct MapRandGenRoomTemplate
{
	// Type of room.
	MapRandGenRoomType::Type type;

	// World bounds in template map.
	Rect2D bounds;

	// Dimensions in tiles.
	int tile_width;
	int tile_height;
	int tile_x;
	int tile_y;

	// Dimensions in room template sizes. (tile_width / template_room_tile_width)
	int room_width;
	int room_height;

	// Maximum instances.
	int max_instances;
	int instance_count;

	// Connecting junctions (essentially doors).
	std::vector<MapRandGenJunctionTemplate> junctions; 

	// Rules that define where this room can be placed.
	std::vector<MapRandGenRoomRule> rules;

	// Objects to be placed in the room.
	std::vector<MapRandGenObjectTemplate> objects;
};

struct MapRandGenRoomConnection
{
	MapRandGenRoomTemplate*		room_template;
	MapRandGenJunctionTemplate* parent_connected_junction;
	MapRandGenJunctionTemplate* room_connected_junction;
};

struct MapRandGenTreeNode
{
	enum 
	{
		max_children = 16,
	};

	MapRandGenTreeNode* parent;

	bool bBoundsCalculated;
	Rect2D bounds;

	MapRandGenRoomTemplate* room_template;
	MapRandGenJunctionTemplate* parent_connected_junction;
	MapRandGenJunctionTemplate* room_connected_junction;

	int children_count;
	MapRandGenTreeNode* children[max_children];

	std::vector<ScriptedActor*> objects;

	std::vector<MapRandGenJunctionTemplate*> locked_junctions;

	bool bPartOfRoot;

	bool bSpawnKey;

	MapRandGenTreeNode()
		: children_count(0)
		, parent(NULL)
		, room_template(NULL)
		, parent_connected_junction(NULL)
		, room_connected_junction(NULL)
		, bBoundsCalculated(false)
		, bPartOfRoot(false)
		, bSpawnKey(false)
	{
	}
};

class MapRandGenManager
{
	MEMORY_ALLOCATOR(MapRandGenManager, "Scene");

public:
	enum 
	{
		// Size of each room template. Map templates must be multiples of these.
		template_room_tile_width = 29,
		template_room_tile_height = 21,

		// Length of the root path in rooms, from start to end.
		// TODO: Scale by difficulty.
		min_root_path_length = 8,
		max_root_path_length = 14,

		// Max number of sub-paths off the root path.
		min_sub_path_count = 1,
		max_sub_path_count = 6,

		// Length of sub paths that branch off from the root path.
		min_sub_path_length = 3,
		max_sub_path_length = 10,

		// Sub path laying reattempts.
		sub_path_place_reattempts = 32,

		// Layer to place collision on.
		collision_layer = 2,

		// Maxmimum number of rooms in a rand-gen map.
		max_rooms = 4096
	};

private:
	Random m_rand;
	std::vector<MapRandGenRoomTemplate> m_room_templates;

	std::vector<MapRandGenRoomRule> m_room_type_rules[MapRandGenRoomType::COUNT];
	std::vector<MapRandGenTreeNode*> m_nodes;

	MapRandGenTemplateTemplate m_template_templates[MapRandGenTemplateType::COUNT];

	StackAllocator<MapRandGenTreeNode, max_rooms> m_node_allocator;

	MapRandGenTreeNode m_root;
	int m_rooms_placed_counts[MapRandGenRoomType::COUNT];

	int m_tile_width;
	int m_tile_height;

	float m_origin_x;
	float m_origin_y;

	Rect2D m_map_bounds;

	void LoadTemplate(MapFile* map_template);

	std::vector<MapRandGenRoomConnection> GetPossibleSpatialConnections(MapRandGenTreeNode* base, std::vector<MapRandGenRoomTemplate*> rooms);
	std::vector<MapRandGenRoomTemplate*> GetPossibleConnectingRoomTemplates(MapRandGenTreeNode* base, int AbsOffsetFromRootFirst, int AbsOffsetFromRootLast, int AbsOffsetFromSubFirst, int AbsOffsetFromSubLast, bool bOnRootPath);
	bool AreNodesOverlapping(MapRandGenTreeNode* a, MapRandGenTreeNode* b);
	Rect2D GetNodeBounds(MapRandGenTreeNode* node);

	void DumpDebugPreview(const char* map_file);

	std::vector<MapRandGenTreeNode*> GeneratePath(MapRandGenTreeNode* base, int length);

	void PasteTilemap(Map* map, std::vector<MapFileLayerBlock*>& layers, int to_x, int to_y, int from_x, int from_y, int width, int height, int only_layer = -1);
	void PasteObjects(Map* map, MapRandGenTreeNode* node, int to_x, int to_y, std::vector<MapRandGenObjectTemplate> objects);

	void RebaseOrigin();

public:	
	MapRandGenManager();

	void Generate(Map* map, MapFile* map_template, int seed);

};

#endif

