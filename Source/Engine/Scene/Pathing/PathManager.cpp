// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scene/Pathing/PathManager.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Renderer/PrimitiveRenderer.h"
#include "Engine/Profiling/ProfilingManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Actor.h"
#include "Engine/Scene/Map/Map.h"
#include "Engine/Scripts/EngineVirtualMachine.h"

#include "Generic/Types/IntVector3.h"

#include "Generic/Threads/Atomic.h"
#include "Generic/Helper/CollisionHelper.h"

#include <float.h>

void PathHandle::Dispose()
{
	if (Is_Valid())
	{
		PathManager* manager = PathManager::Get();

		int index = Atomic::Increment32(&manager->m_pending_delete_path_count) - 1;
		DBG_ASSERT(index < PathManager::max_paths);

		manager->m_pending_delete_paths[index] = *this;

		Invalidate();
	}
}

void PathHandle::Invalidate()
{
	m_path = NULL;
	m_incarnation = -1;
}

bool PathHandle::Is_Complete()
{
	return m_path != NULL && m_path->m_is_complete;
}

bool PathHandle::Has_Path()
{
	return m_path != NULL && m_path->m_valid_path;
}

bool PathHandle::Is_Valid()
{
	return m_path != NULL && m_path->m_incarnation == m_incarnation;
}

PathManager::PathManager()
	: m_show_paths(false)
	, m_running(false)
	, m_path_count(0)
	, m_pending_path_count(0)
	, m_pending_delete_path_count(0)
	, m_worker_index_tracker(0)
	, m_graph(NULL)
	, m_blocker_count(0)
{
	DBG_LOG("sizeof(PathManager) = %i kb.", sizeof(PathManager) / 1024);
	
	m_worker_semaphore = Semaphore::Create();
	m_pending_path_mutex = Mutex::Create();

	m_worker_count = Max(1, Min(max_workers, Platform::Get()->Get_Core_Count() / 2));
	m_running = true;

	for (int i = 0; i < max_paths; i++)
	{
		m_paths[i] = &m_path_memory[i];
	}

	for (int i = 0; i < max_workers; i++)
	{
		PathGenerationState& state = m_generation_states[i];	
		state.Nodes = NULL;
	}

	for (int i = 0; i < m_worker_count; i++)
	{
		m_worker_threads[i] = Thread::Create("PathGenerator", &PathManager::Static_Thread_Entry_Point, this);
		DBG_ASSERT(m_worker_threads[i] != NULL);

		m_worker_threads[i]->Start();
	}
}

PathManager::~PathManager()
{
	m_running = false;
	for (int i = 0; i < m_worker_count; i++)
	{
		m_worker_semaphore->Signal();
	}

	for (int i = 0; i < m_worker_count; i++)
	{
		m_worker_threads[i]->Join();
		SAFE_DELETE(m_worker_threads[i]);
	}

	SAFE_DELETE_ARRAY(m_graph);
	SAFE_DELETE(m_worker_semaphore);
}

void PathManager::Reset()
{
	Pause();

	Regenerate_Graph();

	for (int i = 0; i < max_paths; i++)
	{
		Atomic::Increment32(&m_paths[i]->m_incarnation);
		m_paths[i]->m_pending = false;
		m_paths[i]->m_is_complete = false;
		m_paths[i]->m_valid_path = false;
		m_paths[i]->m_allocated = false;
	}

	m_path_count = 0;
	m_pending_path_count = 0;
	m_pending_delete_path_count = 0;
	m_blocker_count = 0;

	Resume();
}

void PathManager::Regenerate_Graph()
{
	double start_time = Platform::Get()->Get_Ticks();

	// Resize graph.
	Map* map = GameEngine::Get()->Get_Map();
	m_graph_width = map->Get_Width();
	m_graph_height = map->Get_Height();
	m_tile_width = map->Get_Tile_Width();
	m_tile_height = map->Get_Tile_Height();

	SAFE_DELETE_ARRAY(m_graph);
	m_graph = new PathGraphNode[m_graph_width * m_graph_height];

	for (int i = 0; i < max_workers; i++)
	{
		PathGenerationState& state = m_generation_states[i];		
		SAFE_DELETE_ARRAY(state.Nodes);
		state.Nodes = new PathGraphNodeState[m_graph_width * m_graph_height];
	}

	for (int y = 0; y < m_graph_height; y++)
	{
		for (int x = 0; x < m_graph_width; x++)
		{
			PathGraphNode& node = Get_Graph_Node(x, y);// m_graph[(y * m_graph_width) + x];
			node.blocked_on_frame		= 0;
			node.spawn_blocked_on_frame	= 0;
			node.open					= false;
			node.last_used_time			= Platform::Get()->Get_Ticks();
			node.edge_weight			= 0.0f;
			node.tile_position			= IntVector3(x, y, 2);
			node.position				= Vector2((x * m_tile_width) + (m_tile_width * 0.5f), (y * m_tile_height) + (m_tile_height * 0.5f));
		}
	}

	// Flood fill graph with nodes starting at each pylon.
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMLinkedSymbol* pylon_class = vm->Find_Class("Path_Pylon");
	DBG_ASSERT(pylon_class != NULL);

	std::vector<Actor*> pylons = GameEngine::Get()->Get_Scene()->Get_Actors_Of_Class(pylon_class);
	for (std::vector<Actor*>::iterator iter = pylons.begin(); iter != pylons.end(); iter++)
	{
		Actor* pylon = *iter;
		Flood_Fill_Graph(map, pylon);
	}
	
	// Weight all nodes based on distance from collision - prevents
	// "hugging" of walls which looks like ass. Allows boid movement
	// to spread out more as well.
	for (int y = 0; y < m_graph_height; y++)
	{
		for (int x = 0; x < m_graph_width; x++)
		{
			PathGraphNode& node = Get_Graph_Node(x, y);//m_graph[(y * m_graph_width) + x];

			//PathGraphNode* closest_node = NULL;
			//float closest_node_distance = 0.0f;			
			//Find_Closest_Edge(node, (float)max_edge_weight_distance, closest_node, closest_node_distance);

			if (node.open == false || x == 0 || y == 0 || x == m_graph_width - 1 || y == m_graph_height - 1)
			{
				node.is_edge = true;
				node.edge_weight = 1.0f;
			}
			else
			{
				node.is_edge = false;
				node.edge_weight = 0.0f;
			}
		}
	}

	// Crappy-Blur to propogate the edge weight.
	for (int i = 0; i < weight_propogation_iterations; i++)
	{
		for (int y = 0; y < m_graph_height; y++)
		{
			for (int x = 0; x < m_graph_width; x++)
			{
				PathGraphNode& node = Get_Graph_Node(x, y);//m_graph[(y * m_graph_width) + x];
			
				if (node.is_edge == false)
				{
					float sum = 0.0f;
					int sum_cnt = 0;

					for (int ey = -1; ey <= 1; ey++)
					{
						for (int ex = -1; ex <= 1; ex++)
						{
							int rx = x + ex;
							int ry = y + ey;

							if (ex == ey || rx < 0 || ry < 0 || rx >= m_graph_width || ry >= m_graph_height)
							{
								continue;
							}

							PathGraphNode& other_node = Get_Graph_Node(rx, ry);//m_graph[(ry * m_graph_width) + rx];

							sum += other_node.edge_weight;
							sum_cnt++;
						}
					}

					node.edge_weight = sum / (float)sum_cnt;
				}
			}
		}
	}

	DBG_LOG("Regenerated path graph in %.2f ms.", Platform::Get()->Get_Ticks() - start_time);
}

PathGraphNode* PathManager::Find_Closest_Node(Vector3 start)
{
	start.X /= m_tile_width;
	start.Y /= m_tile_height;

	int start_x = (int)start.X;
	int start_y = (int)start.Y;

	if (start_x >= 0 && start_y >= 0 && start_x < m_graph_width && start_y < m_graph_height)
	{
		PathGraphNode& first_node = Get_Graph_Node(start_x, start_y);//m_graph[(start_y * m_graph_width) + start_x];
		if (first_node.open == true)
		{
			return &first_node;
		}
	}

	int distance = 1;
	while (distance < 8)
	{
		bool any_inside = false;

		for (int y = -distance; y <= distance; y++)
		{
			for (int x = -distance; x <= distance; x++)
			{
				int node_x = start_x + x;
				int node_y = start_y + y;

				if (x == y || node_x < 0 || node_y < 0 || node_x >= m_graph_width || node_y >= m_graph_height)
				{
					continue;
				}

				if (x == -distance || y == -distance || x == distance || y == distance)
				{
					any_inside = true;

					PathGraphNode& other_node = Get_Graph_Node(node_x, node_y);//m_graph[(node_y * m_graph_width) + node_x];
					if (other_node.open == true)
					{
						return &other_node;
					}
				}
			}
		}

		if (!any_inside)
		{
			break;
		}

		distance++;
	}

	return NULL;
}

void PathManager::Flood_Fill_Graph(Map* map, Actor* actor)
{
	// Grab seed point.
	int start_layer = actor->Get_Layer();
	int start_x = (int)(actor->Get_Position().X / m_tile_width);
	int start_y = (int)(actor->Get_Position().Y / m_tile_height);

	// Create grid of flags
	bool* tile_flags = new bool[m_graph_width * m_graph_height];
	memset(tile_flags, 0, m_graph_width * m_graph_height * sizeof(bool));

	// Non-recursive flood filling!
	std::vector<FloodFillNode> stack;
	stack.push_back(FloodFillNode(start_x, start_y, 0));
	while (stack.size() > 0)
	{
		FloodFillNode point = stack.at(stack.size() - 1);
		stack.pop_back();

		// Ignore points outside the map (would be more efficient to just not push them!)
		if (point.X < 0 || point.Y < 0 || point.X >= m_graph_width || point.Y >= m_graph_height)
		{
			continue;
		}

		int offset = (point.Y * m_graph_width) + point.X;
		DBG_ASSERT(offset >= 0 && offset < m_graph_width * m_graph_height);

		bool& flag = tile_flags[offset];
		MapTile* tile = map->Get_Tile(point.X, point.Y, start_layer);

		if (flag == false && tile->Collision != MapTileCollisionType::Solid)
		{
			flag = true;

			// Create node!
			m_graph[offset].pylon = actor;
			m_graph[offset].open = true;

			switch (tile->Collision)
			{
			case MapTileCollisionType::Player_Only:
				{
					m_graph[offset].collision = CollisionGroup::Player;
					break;
				}
			case MapTileCollisionType::Enemy_Only:
				{
					m_graph[offset].collision = CollisionGroup::Enemy;
					break;
				}
			default:
				{
					m_graph[offset].collision = CollisionGroup::NONE;
					break;
				}
			}

			if (tile->Collision != MapTileCollisionType::Path_Blocking)
			{
				stack.push_back(FloodFillNode(point.X + 1, point.Y, start_layer));
				stack.push_back(FloodFillNode(point.X - 1, point.Y, start_layer));
				stack.push_back(FloodFillNode(point.X, point.Y + 1, start_layer));
				stack.push_back(FloodFillNode(point.X, point.Y - 1, start_layer));
			}
		}
	}	

	SAFE_DELETE_ARRAY(tile_flags);
}
	
bool PathManager::Get_Show_Paths()
{
	return m_show_paths;
}

void PathManager::Set_Show_Paths(bool toggle)
{
	m_show_paths = toggle;
}
	
void PathManager::Pause()
{
	m_thread_pause_point.Pause(m_worker_count, m_worker_semaphore);
}

void PathManager::Resume()
{
	m_thread_pause_point.Resume();
}

void PathManager::Create_Blocker(Rect2D area, bool spawn_blocking_only)
{
	if (m_blocker_count >= max_blockers)
	{
		DBG_LOG("Ran out of path blockers! Ignoring attempt to create blocker.");
		return;
	}

	m_blockers[m_blocker_count].Area = area;
	m_blockers[m_blocker_count].Spawn_Blocking = spawn_blocking_only;
	m_blocker_count++;
}

/*
void PathManager::Unblock_Nodes()
{
	for (int y = 0; y < m_graph_height; y++)
	{
		for (int x = 0; x < m_graph_width; x++)
		{
			PathGraphNode& node = m_graph[(y * m_graph_width) + x];// Get_Graph_Node(x, y);//m_graph[(y * m_graph_width) + x];
			node.blocked = false;
			node.spawn_blocked = false;
		}
	}
}
*/

void PathManager::Block_Nodes_In_Region(Rect2D area, bool spawn_blocking)
{
	int min_x = (int)floorf(area.X / m_tile_width);
	int min_y = (int)floorf(area.Y / m_tile_height);
	int max_x = (int)floorf((area.X + area.Width) / m_tile_width);
	int max_y = (int)floorf((area.Y + area.Height) / m_tile_height);

	min_x = Clamp(min_x, 0, m_graph_width - 1);
	min_y = Clamp(min_y, 0, m_graph_height - 1);
	max_x = Clamp(max_x, 0, m_graph_width - 1);
	max_y = Clamp(max_y, 0, m_graph_height - 1);

	for (int y = min_y; y <= max_y; y++)
	{
		for (int x = min_x; x <= max_x; x++)
		{
			PathGraphNode& node = Get_Graph_Node(x, y);

			if (spawn_blocking)
			{
				node.spawn_blocked_on_frame = m_frame;
			}
			else
			{
				node.blocked_on_frame = m_frame;
			}
		}
	}
}

void PathManager::Tick(const FrameTime& time)
{
	GameEngine* engine = GameEngine::Get();
	TaskManager* manager = TaskManager::Get();
	CollisionManager* collision_manager = CollisionManager::Get();

	m_frame++;

	// TODO: We should pause while we block/unblock nodes but honestly who gives a fuck
	//		 it will slow us down but won't give us any real benefit. We will get invalid/stale paths
	//		 generated regardless when blockers move.

	{
		PROFILE_SCOPE("Unblock Nodes");

		// Unblock all nodes.
		//Unblock_Nodes(); 
	}

	{
		PROFILE_SCOPE("Block Nodes");

		// Grab active solid collisions out of collision manager and block nodes.
		for (int i = 0; i < m_blocker_count; i++)
		{
			PathBlocker& blocker = m_blockers[i];
			Block_Nodes_In_Region(blocker.Area, blocker.Spawn_Blocking);
		}

		m_blocker_count = 0;
	}

	// Dispose of deleted paths.
	if (m_pending_delete_path_count > 0)
	{
		PROFILE_SCOPE("Deleting Paths");

		Pause();

		for (int i = 0; i < m_pending_delete_path_count; i++)
		{
			PathHandle handle = m_pending_delete_paths[i];
			//DBG_LOG("[PATHS] Deleting Handle: 0x%08x", handle.m_path);

			if (!handle.Is_Valid())
			{
				break;
			}

			Path* path = handle.Get_Path();

			DBG_ASSERT(m_pending_path_count >= 0);

			// Remove path from pending list.
			if (path->m_pending == true)
			{
				for (int j = 0; j < m_pending_path_count; j++)
				{
					if (m_pending_paths[j] == path)
					{
						m_pending_paths[j] = m_pending_paths[m_pending_path_count--];
						break;
					}
				}
			}

			// Remove from path list.
			DBG_ASSERT(m_path_count >= 0);
			DBG_ASSERT(m_pending_path_count >= 0);

			int p_index = path->m_index;
			path->m_allocated = false;

			m_paths[p_index] = m_paths[--m_path_count];
			m_paths[m_path_count] = path;
			m_paths[p_index]->m_index = p_index;
		}
		m_pending_delete_path_count = 0;

		Resume();
	}

	/*
	{
		PROFILE_SCOPE("Weight Nodes");

		// Heavily weight nodes that existing paths are on.
		for (int y = 0; y < m_graph_height; y++)
		{
			for (int x = 0; x < m_graph_width; x++)
			{
				PathGraphNode& node = Get_Graph_Node(x, y);//m_graph[(y * m_graph_width) + x];
				node.path_weight = 0.0f;
			}
		}
		for (int i = 0; i < m_path_count; i++)
		{
			Path* path = m_paths[i];
			if (path->m_pending == false && path->m_valid_path == true && path->m_node_count > 0)
			{
				for (int j = 0; j < path->m_node_count; j++)
				{
					path->m_nodes[j]->path_weight += 0.5f;
				}
			}
		}
	}
	*/
}

void PathManager::Draw(const FrameTime& time)
{
	if (!m_show_paths)
		return;

	PrimitiveRenderer pr;

	int padding = m_tile_width / 4;

	// Draw each node.
	double current_tick = Platform::Get()->Get_Ticks();
	for (int y = 0; y < m_graph_height; y++)
	{
		for (int x = 0; x < m_graph_width; x++)
		{
			PathGraphNode& node = Get_Graph_Node(x, y);//m_graph[(y * m_graph_width) + x];
			if (node.open == true && node.blocked_on_frame < m_frame - 1)
			{
				Rect2D r
				(
					(x * m_tile_width) + padding ,
					(y * m_tile_height) + padding,
					m_tile_width - (padding * 2),
					m_tile_height - (padding * 2)
				);

				float path_weight = 0.0f;
				if (node.last_used_time < current_tick - (double)node_cooldown_time)
				{
					path_weight = 2.0f;
				}

				pr.Draw_Solid_Oval(r, Color(255 * (1.0f - ((node.edge_weight + path_weight) * 0.5f)), 0.0f, 0.0f, 255.0f));
			}
		}
	}

	// Draw all paths.
	for (int i = 0; i < m_path_count; i++)
	{
		Path& path = *m_paths[i];
		if (path.m_pending == false && path.m_valid_path == true && path.m_node_count > 0)
		{
			for (int j = 0; j < path.m_node_count - 1; j++)
			{
				PathGraphNode* from_node = path.m_nodes[j];
				PathGraphNode* to_node = path.m_nodes[j + 1];

				float from_x = (float)(from_node->tile_position.X * m_tile_width) + (m_tile_width * 0.5f);
				float from_y = (float)(from_node->tile_position.Y * m_tile_height) + (m_tile_height * 0.5f);
				float to_x = (float)(to_node->tile_position.X * m_tile_width) + (m_tile_width * 0.5f);
				float to_y = (float)(to_node->tile_position.Y * m_tile_height) + (m_tile_height * 0.5f);

				Rect2D r
				(
					from_x - 4.0f,
					from_y - 4.0f,
					8.0f,
					8.0f
				);

				pr.Draw_Solid_Oval(r, Color(255.0f, 0.0f, 255.0f, 255.0f));
				pr.Draw_Line(Vector3(from_x, from_y, 0.0f), Vector3(to_x, to_y, 0.0f), 2.0f, Color::Green);
			}
		}
	}

	// Draw start/end
	for (int i = 0; i < m_path_count; i++)
	{
		Path& path = *m_paths[i];

		if (path.m_start_node == NULL || path.m_end_node == NULL)
		{
			continue;
		}

		float from_x = (float)(path.m_start_node->tile_position.X * m_tile_width) + (m_tile_width * 0.5f);
		float from_y = (float)(path.m_start_node->tile_position.Y * m_tile_height) + (m_tile_height * 0.5f);
		float to_x = (float)(path.m_end_node->tile_position.X * m_tile_width) + (m_tile_width * 0.5f);
		float to_y = (float)(path.m_end_node->tile_position.Y * m_tile_height) + (m_tile_height * 0.5f);

		Rect2D r
			(
			from_x - 2.0f,
			from_y - 2.0f,
			4.0f,
			4.0f
			);

		pr.Draw_Solid_Oval(r, Color(255.0f, 255.0f, 255.0f, 255.0f));

		Rect2D r2
			(
			to_x - 2.0f,
			to_y - 2.0f,
			4.0f,
			4.0f
			);

		pr.Draw_Solid_Oval(r2, Color(0.0f, 255.0f, 0.0f, 255.0f));
	}

	// Draw debug rects.
	for (int i = 0; i < m_debug_rect_count; i++)
	{
	//	pr.Draw_Solid_Quad(m_debug_rects[i], Color(0.0f, 0.0f, 32.0f * i, 255.0f));
	}
}

float PathManager::Calculate_Heuristic(PathGraphNodeState* from, PathGraphNodeState* to)
{
	float h = (float)abs(to->Node->tile_position.X - from->Node->tile_position.X);
	float v = (float)abs(to->Node->tile_position.Y - from->Node->tile_position.Y);

	float w = 0;
	if (h > v)
	{
		w = diagonal_penalty * v + default_penalty * (h - v);
	}
	else
	{
		w = diagonal_penalty * h + default_penalty * (v - h);
	}

	w += (to->Weight + from->Weight) * edge_penalty;

	return w;
}

float PathManager::Calculate_GScore(PathGraphNodeState* from, PathGraphNodeState* to)
{
	float h = (float)(to->Node->tile_position.X - from->Node->tile_position.X);
	float v = (float)(to->Node->tile_position.Y - from->Node->tile_position.Y);

	if (h != 0.0f && v != 0.0f)
	{
		return diagonal_penalty;
	}
	else
	{
		return default_penalty;
	}
}

PathGraphNodeState* PathManager::Find_Lowest_F_Score(PathGenerationState* state)
{
	// TODO: Ordered queue instead? Save the iteration.

	float lowest_f_score = FLT_MAX;
	PathGraphNodeState* lowest_node = NULL;

	for (int i = 0; i < state->Open_List_Size; i++)
	{
		if (state->Open_List[i]->F_Score < lowest_f_score || lowest_node == NULL)
		{
			lowest_node = state->Open_List[i];
			lowest_f_score = lowest_node->F_Score;
		}
	}

	return lowest_node;
}

void PathManager::Find_Neigbours(PathGenerationState* state, PathGraphNodeState* node, Path* path)
{	
	state->Neighbour_List_Size = 0;

	int base_x = node->Node->tile_position.X;
	int base_y = node->Node->tile_position.Y;

	for (int y = -1; y <= 1; y++)
	{
		for (int x = -1; x <= 1; x++)
		{
			if (x == 0 && y == 0)
			{
				continue;
			}
			
			int node_x = base_x + x;
			int node_y = base_y + y;

			if (node_x < 0 || node_y < 0 || node_x >= m_graph_width || node_y >= m_graph_height)
			{
				continue;
			}

			PathGraphNodeState* neighbour = &state->Nodes[(node_y * m_graph_width) + node_x];
			bool can_walk_on = neighbour->Blocked == false && (neighbour->Collision & path->m_collides_with) == 0;

			if (can_walk_on)
			{
				bool diagonal = (x != 0 && y != 0);
				bool is_cutting_corner = false;

				if (diagonal == true && node_x > 0 && node_y > 0 && node_x < m_graph_width - 1 && node_y < m_graph_height - 1)
				{
					PathGraphNodeState* x_node = &state->Nodes[(node_y * m_graph_width) + (node_x - x)];
					PathGraphNodeState* y_node = &state->Nodes[((node_y - y) * m_graph_width) + node_x];
					
					bool can_walk_on_x_node = x_node->Blocked == false && (x_node->Collision & path->m_collides_with) == 0;
					bool can_walk_on_y_node = y_node->Blocked == false && (y_node->Collision & path->m_collides_with) == 0;

					if (!can_walk_on_x_node || !can_walk_on_y_node)
					{
						is_cutting_corner = true;
					}
				}

				if (is_cutting_corner == false)
				{
					state->Neighbour_List[state->Neighbour_List_Size++] = neighbour;
				}
			}
		}
	}
}

 Actor* PathManager::Get_Pylon(Vector3 position)
{
	PathGraphNode* node = Find_Closest_Node(position);
	if (node != NULL)
	{
		return node->pylon;
	}
	return NULL;
}

void PathManager::Generate_Path(int worker_index, Path* path)
{
	//DBG_LOG("Generating path 0x%08x ...", path);
	double time = Platform::Get()->Get_Ticks();

	PathGenerationState& state = m_generation_states[worker_index];
	state.Open_List_Size = 0;
	state.Intermediate_List_Size = 0;
	state.Neighbour_List_Size = 0;

	// Either start or end is null then we can't generate a path.
	if (path->m_start_node == NULL || path->m_end_node == NULL)
	{
		path->m_valid_path = false; 
	}

	// Generate away!
	else
	{
		double current_tick = Platform::Get()->Get_Ticks();

		// Reset nodes states.
		for (int y = 0; y < m_graph_height; y++)
		{
			for (int x = 0; x < m_graph_width; x++)
			{
				PathGraphNodeState& node = state.Nodes[(y * m_graph_width) + x];
				node.Node = &Get_Graph_Node(x, y);// &m_graph[(y * m_graph_width) + x];
				node.Parent = NULL;

				node.F_Score = 0.0f;
				node.G_Score = 0.0f;
				node.H_Score = 0.0f;

				node.In_Open_List = false;
				node.In_Closed_List = false;

				float path_weight = 0.0f;
				if (node.Node->last_used_time < current_tick - (double)node_cooldown_time)
				{
					path_weight = 0.5f;
				}

				node.Blocked = !node.Node->open || (node.Node->blocked_on_frame >= path->m_start_frame);
				node.Weight = node.Node->edge_weight + path_weight;
				node.Collision = node.Node->collision;
			}
		}

		// TODO: Block nodes with actors on top of them.

		// Grab start and end states.
		PathGraphNodeState* start_state = &state.Nodes[(path->m_start_node->tile_position.Y * m_graph_width) + path->m_start_node->tile_position.X];
		PathGraphNodeState* end_state   = &state.Nodes[(path->m_end_node->tile_position.Y * m_graph_width) + path->m_end_node->tile_position.X];

		// Add start tile to open list.
		start_state->G_Score = 0.0f;
		start_state->H_Score = Calculate_Heuristic(start_state, end_state);
		start_state->F_Score = start_state->H_Score;
		start_state->In_Open_List = true;

		// Find the path.
		state.Open_List[state.Open_List_Size++] = start_state;
		start_state->Open_list_Index = state.Open_List_Size - 1;

		bool found_path = false;

		while (state.Open_List_Size > 0)
		{
			// Find lowest F-Score node.
			PathGraphNodeState* lowest_f = Find_Lowest_F_Score(&state);
			if (lowest_f == end_state)
			{
				found_path = true;
				break;
			}

			// Move from open to closed list.
			lowest_f->In_Open_List = false;
			lowest_f->In_Closed_List = true;
			state.Open_List[lowest_f->Open_list_Index] = state.Open_List[--state.Open_List_Size];
			state.Open_List[lowest_f->Open_list_Index]->Open_list_Index = lowest_f->Open_list_Index;
			lowest_f->Node->touched = true;

			// Find neighbors.
			Find_Neigbours(&state, lowest_f, path);
			for (int i = 0; i < state.Neighbour_List_Size; i++)
			{
				PathGraphNodeState* neighbour = state.Neighbour_List[i];
				if (neighbour->In_Closed_List == true)
				{
					continue;
				}

				float tentative_g_score = lowest_f->G_Score + Calculate_GScore(lowest_f, neighbour);
				bool  tentative_is_better = false;

				if (neighbour->In_Open_List == false)
				{
					neighbour->In_Open_List = true;

					state.Open_List[state.Open_List_Size++] = neighbour;
					neighbour->Open_list_Index = state.Open_List_Size - 1;

					tentative_is_better = true;
				}
				else if (tentative_g_score < neighbour->G_Score)
				{
					tentative_is_better = true;
				}

				if (tentative_is_better == true)
				{
					neighbour->Parent = lowest_f;
					neighbour->G_Score = tentative_g_score;
					neighbour->H_Score = Calculate_Heuristic(neighbour, end_state);
					neighbour->F_Score = neighbour->G_Score + neighbour->H_Score;
				}
			}
		}

		if (found_path == true)
		{
			// Travel back up the path to reconstruct it.
			state.Intermediate_List_Size = 0;
			PathGraphNodeState* node = end_state;
			while (node != NULL)
			{
				state.Intermediate_List[state.Intermediate_List_Size++] = node;
				node = node->Parent;
			}

			// Reverse the list.
			for (int i = 0; i < (state.Intermediate_List_Size / 2); i++)
			{				
				int end_i = (state.Intermediate_List_Size) - (i + 1);

				PathGraphNodeState* tmp = state.Intermediate_List[end_i];
				state.Intermediate_List[end_i] = state.Intermediate_List[i];
				state.Intermediate_List[i] = tmp;
			}

			// Simplify path if possible.
			int non_simplified_count = state.Intermediate_List_Size;
			for (int i = 0; i < state.Intermediate_List_Size - 2; i++)
			{			
				PathGraphNodeState* node_a = state.Intermediate_List[i];
				PathGraphNodeState* node_b = state.Intermediate_List[i + 1];
				PathGraphNodeState* node_c = state.Intermediate_List[i + 2];

				bool v_aligned = node_a->Node->tile_position.Y == node_b->Node->tile_position.Y && node_b->Node->tile_position.Y == node_c->Node->tile_position.Y;
				bool h_aligned = node_a->Node->tile_position.X == node_b->Node->tile_position.X && node_b->Node->tile_position.X == node_c->Node->tile_position.X;

				// Aligned in either axis? Remove node B.
				if (v_aligned == true || h_aligned == true)
				{
					// Shift everything back to overwrite node B.
					for (int j = i + 1; j < state.Intermediate_List_Size - 1; j++)
					{
						state.Intermediate_List[j] = state.Intermediate_List[j + 1];
					}

					state.Intermediate_List_Size--;
					i--;
				}
			}
			
			// Move simplified nodes into final path.
			path->m_node_count = Min(Path::max_nodes_in_path, state.Intermediate_List_Size);
			for (int i = 0; i < path->m_node_count; i++)
			{
				path->m_nodes[i] = state.Intermediate_List[i]->Node;
				path->m_nodes[i]->last_used_time = current_tick;
			}
		}

		path->m_valid_path = found_path;
	}

	path->m_pending = false;
	path->m_is_complete = true;

	double elapsed = Platform::Get()->Get_Ticks() - time;
	if (elapsed > 1.0f)
	{
		DBG_LOG("Path 0x%08x took %.2f ms to generate ...", path, elapsed);
	}
}

void PathManager::Thread_Entry_Point(Thread* self, void* ptr)
{
	int worker_index = Atomic::Increment32(&m_worker_index_tracker) - 1;

	while (m_running == true)
	{	
		// Wait for some paths before we start running.
		if (m_pending_path_count == 0)
		{
			m_worker_semaphore->Wait();
		}

		// Pause if required.
		m_thread_pause_point.Poll();

		// Anything to actually do?
		if (m_pending_path_count == 0 || m_running == false)
		{
			continue;
		}

		// Grab next path and start running it.	
		while (m_running == true)
		{
			if (m_pending_path_count > 0)
			{
				Path* path = NULL;

				{
					MutexLock lock(m_pending_path_mutex);
					if (m_pending_path_count > 0)
					{
						path = m_pending_paths[--m_pending_path_count];
					}
				}

				if (path != NULL)
				{
					DBG_ASSERT(path != NULL);
					Generate_Path(worker_index, path);
				}
			}
			else
			{
				break;
			}
		}
	}
}

PathHandle PathManager::Create_Handle(
	Vector3& start,
	Vector3& end,
	CollisionGroup::Type collides_with
)
{
	PathGraphNode* start_node = Find_Closest_Node(start);
	PathGraphNode* end_node = Find_Closest_Node(end);

	int handle_index = Atomic::Increment32(&m_path_count) - 1;

	Path* handle = m_paths[handle_index];
	DBG_ASSERT(!handle->m_allocated);

	//DBG_LOG("[PATHS] New Handle: 0x%08x (%i)", handle, m_path_count);
	
	Atomic::Increment32(&handle->m_incarnation);
	handle->m_index				= handle_index;
	handle->m_start_node		= start_node;
	handle->m_end_node			= end_node;
	handle->m_collides_with		= collides_with;
	handle->m_pending			= true;
	handle->m_is_complete		= false;
	handle->m_valid_path		= false;
	handle->m_allocated			= true;
	handle->m_start_frame		= m_frame;

	{
		MutexLock lock(m_pending_path_mutex);

		DBG_ASSERT(m_pending_path_count >= 0);
		int active_handle_index = m_pending_path_count++;
		DBG_ASSERT(active_handle_index >= 0);
		DBG_ASSERT(m_pending_path_count >= 0);

		m_pending_paths[active_handle_index] = handle;
	}

	m_worker_semaphore->Signal();

	PathHandle h;
	h.m_path = handle;
	h.m_incarnation = handle->m_incarnation;

	return h;
}
