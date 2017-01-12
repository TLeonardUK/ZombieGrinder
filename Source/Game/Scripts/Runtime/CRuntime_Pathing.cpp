// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "Game/Scripts/Runtime/CRuntime_Pathing.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Runner/Game.h"

#include "Generic/Math/Random.h"
#include "Generic/Helper/PersistentLogHelper.h"

#include "Engine/Scene/Pathing/PathManager.h"
#include "Game/Scene/Actors/ScriptedActor.h"

#include "Game/Network/GameNetUser.h"
#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetServer.h"
#include "Game/Network/GameNetClient.h"

CVMObjectHandle CRuntime_Pathing::Get_Pylon_By_Position(CVirtualMachine* vm, CVMObjectHandle position)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	Actor* actor = PathManager::Get()->Get_Pylon(position.Get()->To_Vec3());
	if (actor == NULL)
	{
		return NULL;
	}
	return static_cast<ScriptedActor*>(actor)->Get_Script_Object();
}

struct PathSpawnRect
{
	int x;
	int y;
	int w;
	int h;
	bool valid;
};

CVMObjectHandle CRuntime_Pathing::Find_Random_Spawn_Area(CVirtualMachine* vm, CVMObjectHandle bounding_box, float min_radius, float max_radius, int can_spawn_on_screen)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	double start_ticks = Platform::Get()->Get_Ticks();

	// Static allocation of spawn-point info to save some alloc/free'ing.
	static int MAX_PATH_RECTS = 2048;
	static const int MAX_CAMERAS = 128;
	static const float CAMERA_BUFFER = 64.0f;
	static PathSpawnRect* s_rects = NULL;
	static Rect2D* s_usable_rects = NULL;
	static Rect2D* s_preferred_rects = NULL;
	static Rect2D s_camera_rects[MAX_CAMERAS];
	static Vector2 s_camera_centers[MAX_CAMERAS];
	static PathSpawnRect** s_grid = NULL;
	static int s_grid_width = 0;
	static int s_grid_height = 0;

	bool is_valid = false;
	Rect2D spawn_area;
	int rect_count = 0;
	int usable_rect_count = 0;
	int camera_count = 0;

	float min_radius_sqr = min_radius * min_radius;
	float max_radius_sqr = max_radius * max_radius;

	PathManager* manager = PathManager::Get();
	int graph_width = manager->Get_Graph_Width();
	int graph_height = manager->Get_Graph_Height();
	int tile_width = manager->Get_Tile_Width();
	int tile_height = manager->Get_Tile_Height();

	Vector4 spawn_bounding_box = bounding_box.GetNullCheck(vm)->To_Vec4();

	GameNetManager* net_manager = GameNetManager::Get();
	std::vector<GameNetUser*> users = net_manager->Get_Game_Net_Users();

	// Resize grid.
	if (s_grid_width != graph_width ||
		s_grid_height != graph_height ||
		s_grid == NULL)
	{
		SAFE_DELETE_ARRAY(s_grid);
		SAFE_DELETE_ARRAY(s_rects);
		SAFE_DELETE_ARRAY(s_usable_rects);
		SAFE_DELETE_ARRAY(s_preferred_rects);

		s_grid_width = graph_width;
		s_grid_height = graph_height;

		MAX_PATH_RECTS = s_grid_width * s_grid_height;

		DBG_LOG("Resizing path spawning grid to %i x %i.", s_grid_width, s_grid_height);
		s_grid = new PathSpawnRect*[s_grid_width * s_grid_height];
		s_rects = new PathSpawnRect[MAX_PATH_RECTS];
		s_usable_rects = new Rect2D[MAX_PATH_RECTS];
		s_preferred_rects = new Rect2D[MAX_PATH_RECTS];
	}

	// Clear out grid.
	memset(s_grid, 0, sizeof(PathSpawnRect*) * s_grid_width * s_grid_height);

	// Find center of screen.
	for (std::vector<GameNetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
	{
		GameNetUser* user = *iter;
		Rect2D view = user->Get_Estimated_Viewport();
		if (user->Get_Controller()) // Only give a shit about players controlling chars, spectators and gtfo.
		{
			s_camera_rects[camera_count] = view.Inflate(CAMERA_BUFFER, CAMERA_BUFFER);
			s_camera_centers[camera_count] = view.Center();
			camera_count++;
		}
	}

	//Rect2D camera_bb = Game::Get()->Get_Camera(CameraID::Game)->Get_Bounding_Viewport();
	//Vector2 camera_center = camera_bb.Center();

	int min_x = 0;
	int min_y = 0;
	int max_x = graph_width - 1;
	int max_y = graph_height - 1;

	/*for (int c = 0; c < camera_count; c++)
	{
		Rect2D cr = s_camera_rects[camera_count];

		// Find grid sub-section we care about.
		int min_x = (int)Max(0, (cr.X - max_radius) / tile_width);
		int min_y = (int)Max(0, (cr.Y - max_radius) / tile_height);
		int max_x = (int)Min(graph_width - 1, (cr.X + cr.Width + max_radius) / tile_width);
		int max_y = (int)Min(graph_height - 1, (cr.Y + cr.Height + max_radius) / tile_height);
		*/
		for (int y = min_y; y <= max_y && rect_count < MAX_PATH_RECTS; y++)
		{
			for (int x = min_x; x <= max_x && rect_count < MAX_PATH_RECTS; x++)
			{
				PathGraphNode* node = &manager->Get_Graph_Node(x, y);

				// TODO: Check encroachment of objects.

				// Check its a valid node.
				if (node->open == true && 
					node->blocked_on_frame < PathManager::Get()->Get_Frame() - 1 && 
					node->spawn_blocked_on_frame < PathManager::Get()->Get_Frame() - 1 &&
					node->pylon != NULL && node->pylon->Get_Activated_By_Player())
				{
					bool is_inside_camera = false;
					bool is_inside_radius = false;

					// Inside min/max radius?
					for (int c = 0; c < camera_count; c++)
					{
						if (can_spawn_on_screen == 0)
						{
							is_inside_camera = s_camera_rects[c].Intersects(Rect2D(x * tile_width, y * tile_height, tile_width, tile_height));
							if (is_inside_camera == true)
							{
								break;
							}
						}

						if (is_inside_radius == false)
						{
							float sqr_distance = (s_camera_centers[c] - Vector2(x * tile_width, y * tile_height)).Length_Squared();
							//float sqr_distance = //s_camera_rects[c].DistanceToSquared(Vector2(x * tile_width, y * tile_height));
							/*
							// Sqr_distance means inside,
							if (sqr_distance == 0.0f)
							{

							}
							*/
							if (sqr_distance >= min_radius_sqr &&
								sqr_distance <= max_radius_sqr)
							{
								is_inside_radius = true;
							}
						}
					}

					if (is_inside_camera == false &&
						is_inside_radius == true)
					{
						// Add node to list.
						PathSpawnRect* output_rect = &s_rects[rect_count++];
						output_rect->x = x;
						output_rect->y = y;
						output_rect->w = 1;
						output_rect->h = 1;
						output_rect->valid = true;
						s_grid[(graph_width * y) + x] =  output_rect;
					}
				}
			}
		}
	//}

	// Condense into multiple rects vertically.
	for (int i = 0; i < rect_count; i++)
	{
		PathSpawnRect* base_rect = &s_rects[i]; 

		// Rect still valid?
		if (!base_rect->valid)
		{
			continue;
		}

		// Merge vertically.
		for (int y = base_rect->y + 1; y <= max_y; y++)
		{
			PathSpawnRect** merge_rect = &s_grid[(graph_width * y) + base_rect->x];
			if (*merge_rect != NULL)
			{
				base_rect->h++;
				(*merge_rect)->valid = false;
				*merge_rect = base_rect;
			}
			else
			{
				break;
			}			
		}
	}
	
	// Now horizontally.
	for (int i = 0; i < rect_count; i++)
	{
		PathSpawnRect* base_rect = &s_rects[i]; 

		// Rect still valid?
		if (!base_rect->valid)
		{
			continue;
		}

		// Merge horizontally.
		for (int x = base_rect->x + 1; x <= max_x; x++)
		{
			PathSpawnRect** merge_rect = &s_grid[(graph_width * base_rect->y) + x];
			if (*merge_rect != NULL && 
				(*merge_rect)->h == base_rect->h &&
				(*merge_rect)->y == base_rect->y)
			{
				base_rect->w++;
				(*merge_rect)->valid = false;

				for (int y = base_rect->y; y < base_rect->h; y++)
				{
					PathSpawnRect** overwrite_rect = &s_grid[(graph_width * y) + x];
					*overwrite_rect = base_rect;
				}
			}
			else
			{
				break;
			}			
		}
	}

	manager->Reset_Debug_Rects();

	// Grab only rects we can fit the bounding box inside of.
	for (int i = 0; i < rect_count; i++)
	{
		PathSpawnRect* base_rect = &s_rects[i]; 

		// Convert to pixel-coordinates.
		base_rect->x *= tile_width;
		base_rect->y *= tile_height;
		base_rect->w *= tile_width;
		base_rect->h *= tile_height;

		// Is usable?
		if (base_rect->valid && 
			base_rect->w >= spawn_bounding_box.Z && 
			base_rect->h >= spawn_bounding_box.W)
		{
			Rect2D output_area
			(
				base_rect->x,
				base_rect->y,
				base_rect->w,
				base_rect->h
			);

			if (output_area.Width >= spawn_bounding_box.Z && 
			    output_area.Height >= spawn_bounding_box.W)
			{
				s_usable_rects[usable_rect_count++] = output_area;

				manager->Add_Debug_Rect(output_area);
			}
		}
	}

	// Grab a random position from a random rect.
	if (usable_rect_count > 0)
	{
		is_valid = true;

		// Look for preferred rects (ones in-front of the players heading).
		std::vector<GameNetUser*> netusers = GameNetManager::Get()->Get_Game_Net_Users();

		int preferred_rect_count = 0;

		for (int i = 0; i < usable_rect_count; i++)
		{
			bool infront = false;

			Vector3 center = Vector3(s_usable_rects[i].Center().X, s_usable_rects[i].Center().Y, 0.0f);

			for (std::vector<GameNetUser*>::iterator iter = netusers.begin(); iter != netusers.end(); iter++)
			{
				GameNetUser* user = *iter;
				if (user->Get_Heading().Length() > 0.05f)
				{
					Vector3 heading = user->Get_Heading();
					Vector3 last_heading_position = user->Get_Heading_Position();
					Vector3 to_player_heading = (last_heading_position - center).Normalize();
					float dot = heading.Dot(to_player_heading);
					if (dot < -0.5f)
					{
						infront = true;
						break;
					}
				}
			}

			if (infront == true)
			{
				s_preferred_rects[preferred_rect_count++] = s_usable_rects[i]; 
			}
		}

		if (preferred_rect_count > 0)
		{
			spawn_area = s_preferred_rects[Random::Static_Next(0, preferred_rect_count)];	
		}
		else
		{
			spawn_area = s_usable_rects[Random::Static_Next(0, usable_rect_count)];	
		}
	}

	DBG_ONSCREEN_LOG(StringHelper::Hash("SpawnGridLog1"), Color::Green, 10.0f, "Rects=%i Time=%.2f", rect_count, Platform::Get()->Get_Ticks() - start_ticks);

	// Return the result!
	CVMLinkedSymbol* path_spawn_point_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);
	CVMObjectHandle result = vm->New_Object(path_spawn_point_class, false);
	CVMObject* obj = result.Get();
	obj->Resize(2);
	obj->Get_Slot(0).int_value = is_valid;
	obj->Get_Slot(1).object_value = vm->Create_Vec4(spawn_area.X, spawn_area.Y, spawn_area.Width, spawn_area.Height);

	return result;
}

void CRuntime_Pathing::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle>("Pathing", "Get_Pylon_By_Position", &Get_Pylon_By_Position);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle,float,float,int>("Pathing", "Find_Random_Spawn_Area", &Find_Random_Spawn_Area);
}
