// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_PATHING_PATHMANAGER_
#define _ENGINE_SCENE_PATHING_PATHMANAGER_

#include "Engine/Engine/FrameTime.h"
#include "Generic/Patterns/Singleton.h"
#include "Engine/Tasks/TaskManager.h"

#include "Generic/Threads/ThreadPausePoint.h"
#include "Generic/Threads/Semaphore.h"

#include "Engine/Scene/Map/Map.h"

#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Vector3.h"
#include "Generic/Types/IntVector3.h"

#include "Generic/Math/Math.h"

class Actor;
class Map;
class PathManager;
struct PathHandle;
class Actor;

struct PathGraphNode
{
	MEMORY_ALLOCATOR(PathGraphNode, "Scene");

public:
	Actor*						pylon;

	bool						is_edge;
	bool						open;
	IntVector3					tile_position;
	Vector2						position;
	CollisionGroup::Type		collision;
	//float						edge_distance;
	float						edge_weight;
	//float						path_weight;

	double						last_used_time;

	unsigned int				blocked_on_frame;
	unsigned int				spawn_blocked_on_frame;

	bool						touched;

};

struct PathGraphNodeState
{
	MEMORY_ALLOCATOR(PathGraphNodeState, "Scene");

public:
	PathGraphNode* Node;
	PathGraphNodeState* Parent;

	float F_Score;
	float G_Score;
	float H_Score;

	bool  In_Open_List;
	bool  In_Closed_List;
	int	  Open_list_Index;

	bool						Blocked;
	CollisionGroup::Type		Collision;
	float						Weight;
	
};

struct PathGenerationState
{
	MEMORY_ALLOCATOR(PathGenerationState, "Scene");

public:
	enum 
	{
		max_open_list_size = 512 * 512, // If our open list is larger than this, we're in enough trouble.
		max_neighbour_size = 8,
		max_intermediate_path_node_count = 512,
	};

	PathGraphNodeState* Nodes;

	int					Open_List_Size;
	PathGraphNodeState* Open_List[max_open_list_size];

	int					Neighbour_List_Size;
	PathGraphNodeState* Neighbour_List[max_neighbour_size];

	int					Intermediate_List_Size;
	PathGraphNodeState*	Intermediate_List[max_intermediate_path_node_count];
};

struct Path
{
	MEMORY_ALLOCATOR(Path, "Scene");

private:
	enum
	{
		max_nodes_in_path = 50
	};

	friend class PathManager;
	friend struct PathHandle;

	int						m_index;
	int						m_incarnation;
	PathGraphNode*			m_start_node;
	PathGraphNode*			m_end_node;
	CollisionGroup::Type	m_collides_with;
	bool					m_pending;
	bool					m_is_complete;
	bool					m_valid_path;
	int						m_node_count;
	PathGraphNode*			m_nodes[max_nodes_in_path];
	bool					m_allocated;
	unsigned int			m_start_frame;

public:
	int Get_Nodes(PathGraphNode**& nodes)
	{
		nodes = m_nodes;
		return m_node_count;
	}

};

struct PathHandle
{
	MEMORY_ALLOCATOR(Path, "Scene");

private:
	friend class PathManager;

	int		m_incarnation;
	Path*	m_path;

public:
	PathHandle()
		: m_incarnation(0)
		, m_path(NULL)
	{
	}

	void Dispose();
	void Invalidate();
	bool Is_Complete();
	bool Is_Valid();
	bool Has_Path();

	Path* Get_Path()
	{
		DBG_ASSERT(Is_Valid());
		return m_path;
	}

};

class PathManager : 
	public Singleton<PathManager>
{
	MEMORY_ALLOCATOR(PathManager, "Scene");

private:
	struct FloodFillNode
	{
		int X, Y, Z;

		FloodFillNode(int x, int y, int z)
			: X(x)
			, Y(y)
			, Z(z)
		{
		}
	};

	struct PathBlocker
	{
		Rect2D	Area;
		bool	Spawn_Blocking;
	};

	bool m_show_paths;

	enum 
	{
		max_workers			= 2,
		max_paths			= 300,

		weight_propogation_iterations = 3,
		
		default_penalty	 = 10,
		diagonal_penalty = 14,
		edge_penalty	 = 100,

		max_debug_rects		= 128,

		max_blockers		= 512,

		node_cooldown_time	= 2000
	};

	Thread* m_worker_threads[max_workers];
	int m_worker_count;

	Path m_path_memory[max_paths];

	Path* m_paths[max_paths];
	int m_path_count;

	PathGenerationState m_generation_states[max_workers];

	Path* m_pending_paths[max_paths];
	int m_pending_path_count;

	Mutex* m_pending_path_mutex;

	PathHandle m_pending_delete_paths[max_paths];
	int m_pending_delete_path_count;

	PathGraphNode* m_graph;
	int m_graph_width;
	int m_graph_height;
	int m_tile_width;
	int m_tile_height;

	bool m_running;

	bool m_paused;
	ThreadPausePoint m_thread_pause_point;

	int m_worker_index_tracker;

	Semaphore* m_worker_semaphore;

	int m_debug_rect_count;
	Rect2D m_debug_rects[max_debug_rects];

	PathBlocker m_blockers[max_blockers];
	int m_blocker_count;

	unsigned int m_frame;

protected:
	friend class PathGenerationTask;
	friend struct PathHandle;

	void Simulate(int worker_index);

	static void Static_Thread_Entry_Point(Thread* self, void* ptr)
	{
		reinterpret_cast<PathManager*>(ptr)->Thread_Entry_Point(self, ptr);
	}
	void Thread_Entry_Point(Thread* self, void* ptr);

	PathGraphNode* Find_Closest_Node(Vector3 start);

	float Calculate_Heuristic(PathGraphNodeState* from, PathGraphNodeState* to);
	float Calculate_GScore(PathGraphNodeState* from, PathGraphNodeState* to);
	PathGraphNodeState* Find_Lowest_F_Score(PathGenerationState* state);
	void Find_Neigbours(PathGenerationState* state, PathGraphNodeState* node, Path* path);

	void Regenerate_Graph();
	void Flood_Fill_Graph(Map* map, Actor* actor);

	void Generate_Path(int worker_index, Path* path);

	void Pause();
	void Resume();

	//void Unblock_Nodes();
	void Block_Nodes_In_Region(Rect2D area, bool spawn_blocking);

public:
	PathManager();
	~PathManager();

	void Reset();

	INLINE unsigned int Get_Frame()
	{
		return m_frame;
	}

	INLINE int Get_Tile_Width()
	{
		return m_tile_width;
	}

	INLINE int Get_Tile_Height()
	{
		return m_tile_height;
	}

	INLINE int Get_Graph_Width()
	{
		return m_graph_width;
	}

	INLINE int Get_Graph_Height()
	{
		return m_graph_height;
	}

	INLINE PathGraphNode& Get_Graph_Node(int x, int y)
	{
		DBG_ASSERT(x >= 0 && y >= 0 && x < m_graph_width && y < m_graph_height);
		return m_graph[(y * m_graph_width) + x];
	}

	void Reset_Debug_Rects()
	{
		m_debug_rect_count = 0;
	}

	void Add_Debug_Rect(Rect2D area)
	{
		if (m_debug_rect_count >= max_debug_rects)
		{
			return;
		}
		m_debug_rects[m_debug_rect_count++] = area;
	}

	PathHandle Create_Handle(
		Vector3& start,
		Vector3& end,
		CollisionGroup::Type collides_with
	);

	Actor* Get_Pylon(Vector3 position);

	void Create_Blocker(Rect2D area, bool spawn_blocking_only = false);

	bool Get_Show_Paths();
	void Set_Show_Paths(bool toggle);

	void Tick(const FrameTime& time);
	void Draw(const FrameTime& time);

};

#endif

