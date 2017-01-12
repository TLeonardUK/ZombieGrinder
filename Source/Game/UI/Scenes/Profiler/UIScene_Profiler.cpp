// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Profiler/UIScene_Profiler.h"
#include "Game/UI/Scenes/UIScene_Game.h"
#include "Game/Network/Packets/GamePackets.h"

#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIComboBox.h"
#include "Engine/UI/Elements/UIMenuBar.h"
#include "Engine/UI/Elements/UISplitContainerItem.h"
#include "Engine/UI/Elements/UIToolbarItem.h"
#include "Engine/UI/Elements/UIMenuItem.h"
#include "Engine/UI/Elements/UIPropertyGrid.h"
#include "Engine/UI/Elements/UICurveEditor.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UIFadeTopLevelTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"

#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/Renderer/RenderPipeline.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scripts/EngineVirtualMachine.h"
#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetClient.h"

#include "Engine/Profiling/ProfilingManager.h"

#include "Game/Runner/Game.h"
#include "Game/Game/EditorManager.h"

#include "Generic/Stats/Stats.h"

#include "Generic/Math/Math.h"
#include "Generic/Types/MemoryHeap.h"
#include "Generic/Types/PooledMemoryHeap.h"
#include "Generic/Helper/StringHelper.h"

#include <cfloat>

UIScene_Profiler::UIScene_Profiler()
	: m_page(0)
	, m_persist_nodes_time(0.0f)
{
	Set_Layout("profiler");

	m_manager		= GameEngine::Get()->Get_UIManager();
	m_font			= m_manager->Get_Font();
	m_font_renderer	= FontRenderer(m_font, false, false);
}

UIScene_Profiler::~UIScene_Profiler()
{
}

UIScene* UIScene_Profiler::Get_Background(UIManager* manager)
{
	return manager->Retrieve_Persistent_Scene("UIScene_GameBackground");
}

const char* UIScene_Profiler::Get_Name()
{
	return "UIScene_Profiler";
}

bool UIScene_Profiler::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_Profiler::Should_Display_Cursor()
{
	return true;
}
	
bool UIScene_Profiler::Should_Display_Focus_Cursor()
{
	return false;
}

bool UIScene_Profiler::Is_Focusable()
{
	return true;
}

bool UIScene_Profiler::Should_Fade_Cursor()
{
	return false;
}

void UIScene_Profiler::Enter(UIManager* manager)
{
}	

void UIScene_Profiler::Exit(UIManager* manager)
{
}

void UIScene_Profiler::Refresh(UIManager* manager)
{
	UIScene::Refresh(manager);
	//Refresh_State();
	//Refresh_Stats();
}

void UIScene_Profiler::Get_Stat_String(UIPropertyGridItem* item, void* value)
{
	//Statistic* stat = (Statistic*)item->Meta_Data;
	//*((std::string*)value) = stat->To_String();
}

void UIScene_Profiler::Set_Stat_String(UIPropertyGridItem* item, void* value)
{
	// Do nothing.
}

void UIScene_Profiler::Add_Stats(UIPropertyGrid* grid, UIPropertyGridItem* parent, std::string path, int depth)
{
	/*
	Statistic** stats;
	int stats_count = StatisticManager::Get_Statistics(stats);

	// Add groups.
	std::vector<std::string> groups;
	for (int i = 0; i < stats_count; i++)
	{
		Statistic& stat = *stats[i];
		if ((int)stat.Path.size() > depth + 1)
		{
			std::string sub_path = StringHelper::Join(stat.Path.begin(), stat.Path.begin() + depth, "/");
			if (sub_path == path)
			{
				std::string group = stat.Path.at(depth);
				if (std::find(groups.begin(), groups.end(), group) == groups.end())
				{
					groups.push_back(group);

					UIPropertyGridItem* item = grid->Add_Custom_Item(group, parent, UIPropertyGridDataType::None, &UIScene_Profiler::Get_Stat_String, Set_Stat_String, std::vector<std::string>(), &stat);
					Add_Stats(grid, item, path == "" ? group : path + "/" + group, depth + 1);
				}
			}
		}
	}

	// Add stats.
	for (int i = 0; i < stats_count; i++)
	{
		Statistic& stat = *stats[i];
		if ((int)stat.Path.size() == depth + 1)
		{
			std::string sub_path = StringHelper::Join(stat.Path.begin(), stat.Path.begin() + depth, "/");
			if (sub_path == path)
			{ 
				std::string name = stat.Path.at(depth);
				grid->Add_Custom_Item(name, parent, UIPropertyGridDataType::String, Get_Stat_String, Set_Stat_String, std::vector<std::string>(), &stat);
			}
		}
	}*/
}

void UIScene_Profiler::Refresh_State()
{
	// Update toolbar.
	//Find_Element<UIToolbarItem*>("memory_toolbar_item")->Set_Selected(m_page == 0);
	//Find_Element<UIToolbarItem*>("cpu_toolbar_item")->Set_Selected(m_page == 1);
	//Find_Element<UIToolbarItem*>("network_toolbar_item")->Set_Selected(m_page == 2);

	// Update page.
	//UIPropertyGrid* grid = Find_Element<UIPropertyGrid*>("properties_panel");
	//grid->Clear_Items();

	// Add stats graph.
	//Add_Stats(grid, NULL, "", 0);

	/*
	switch (m_page)
	{
	// Memory.
	case 0:
		{
			UIPropertyGridItem* group = grid->Add_Item("Memory", NULL);
				grid->Add_Item("RAM Allocated", group, UIPropertyGridDataType::String, &m_stats.ram_allocation);
				grid->Add_Item("RAM Allocs Per Frame", group, UIPropertyGridDataType::int, &m_stats.ram_allocs_per_frame);
				grid->Add_Item("GC Allocated", group, UIPropertyGridDataType::String, &m_stats.gc_allocation);
				grid->Add_Item("GC Allocs Per Frame", group, UIPropertyGridDataType::int, &m_stats.gc_allocs_per_frame);

			UIPropertyGridItem* group = grid->Add_Item("Scene", NULL);
				grid->Add_Item("Total Script Objects", group, UIPropertyGridDataType::String, &m_stats.ram_allocation);
				grid->Add_Item("Total Script Contexts", group, UIPropertyGridDataType::String, &m_stats.gc_allocation);

			break;
		}
	
	// CPU
	case 1:
		{
			break;
		}

	// Network.
	case 2:
		{
			break;
		}
	}*/

}

void UIScene_Profiler::Refresh_Stats()
{
//	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
//	CGarbageCollector* gc = vm->Get_GC();

	//m_stats.ram_allocation = StringHelper::Format("%.2f MB / %.2f MB", (float)platform_get_used_memory() / 1024.0f / 1024.0f, (float)platform_get_malloc_heap()->Get_Total() / 1024.0f / 1024.0f);
//	m_stats.gc_allocation  = StringHelper::Format("%.2f MB / %.2f MB", (float)gc->Get_Total_Used() / 1024.0f / 1024.0f, (float)gc->Get_Total() / 1024.0f / 1024.0f);
}

void UIScene_Profiler::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{		
	// Close the profiler.
	if (Input::Get()->Was_Pressed(InputBindings::Keyboard_F1))
	{
		manager->Play_UI_Sound(UISoundType::Small_Click);
		manager->Go(UIAction::Pop());
		return;
	}

	// Capture next frame.
	ProfilingManager::Capture_Frame(max_profile_depth);

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Profiler::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);

	float res_width = (float)GfxDisplay::Get()->Get_Width();
	float res_height = (float)GfxDisplay::Get()->Get_Height();

	// Draw some stats.
	PrimitiveRenderer pr;
	MarkupFontRenderer tr(manager->Get_Font(), false, false);

	Rect2D bounds = Rect2D(
		10.0f,
		10.0f, 
		(float)line_width, 
		(float)line_height
		);

	Draw_Stats(pr, tr, "", 0, bounds);

	return;

	// Draw a profiler graph at the bottom.
	Rect2D graph_bounds = Rect2D(
		10.0f,
		res_height - profile_graph_height - 10.0f, 
		res_width - 20.0f,
		(float)profile_layer_height
	);

	// Grab list of all captured nodes.
	ProfileFrameCaptureNode* nodes = NULL;
	int node_count = ProfilingManager::Get_Frame_Capture_Nodes(nodes);
	std::vector<Thread*> threads;
	bool disablePersist = false;;

	if (m_persist_nodes_time <= 0.0f)
	{
		m_persist_nodes_count = node_count;
		m_persist_nodes_data = nodes;
	}
	else
	{
		m_persist_nodes_time -= GameEngine::Get()->Get_Time()->Get_Delta_Seconds();
		if (m_persist_nodes_time <= 0.0f)
		{
			SAFE_DELETE(m_persist_nodes_data);

			m_persist_nodes_count = node_count;
			m_persist_nodes_data = nodes;
		}
		else
		{
			disablePersist = true;
		}
	}

	// Work out how many threads we captured.
	double min_time = FLT_MAX;
	double max_time = FLT_MIN;
	for (int i = 0; i < m_persist_nodes_count; i++)
	{
		ProfileFrameCaptureNode& node = m_persist_nodes_data[i];
		if (node.Complete)
		{
			if (std::find(threads.begin(), threads.end(), node.Active_Thread) == threads.end())
			{
				threads.push_back(node.Active_Thread);
			}

			if (node.Start_Time < min_time)
			{
				min_time = node.Start_Time;
			}
			if (node.End_Time > max_time)
			{
				max_time = node.End_Time;
			}
		}
	}

	double range = max_time - min_time;

	if (range > max_frame_time && !disablePersist)
	{
		m_persist_nodes_time = persist_over_time_frames_for;
		m_persist_nodes_count = node_count;
		m_persist_nodes_data = new ProfileFrameCaptureNode[node_count];
		memcpy(m_persist_nodes_data, nodes, sizeof(ProfileFrameCaptureNode) * node_count);
	}

	if (threads.size() > 0)
	{
		for (std::vector<Thread*>::iterator iter = threads.begin(); iter != threads.end(); iter++)
		{
			Thread* thread = *iter;

			pr.Draw_Solid_Quad(graph_bounds, Color(0.0f, 0.0f, 0.0f, 200.0f));
			tr.Draw_String(StringHelper::Format("%s (%.2f ms)", thread->Get_Name().c_str(), range).c_str(), graph_bounds, line_height, Color(255.0f, 0.0f, 0.0f, 255.0f), TextAlignment::Center);

			int max_depth = 0;

			for (int i = 0; i < m_persist_nodes_count; i++)
			{
				ProfileFrameCaptureNode& node = m_persist_nodes_data[i];
				if (node.Complete && node.Active_Thread == thread)
				{
					double start_delta = (node.Start_Time - min_time) / range;
					double end_delta = (node.End_Time - min_time) / range;

					if (node.Depth > max_depth)
					{
						max_depth = node.Depth;
					}

					Rect2D block = Rect2D
					(
						graph_bounds.X + (graph_bounds.Width * (float)start_delta),
						graph_bounds.Y + ((line_height + line_spacing) * (node.Depth + 1)),
						graph_bounds.Width * (float)(end_delta - start_delta),
						(float)line_height
					);

					pr.Draw_Solid_Quad(block, Color(0.0f, 0.0f, 255.0f, 200.0f));

					if (block.Width > 20.0f)
					{	
						tr.Draw_String(node.Name, block, line_height, Color(0.0f, 255.0f, 0.0f, 255.0f), TextAlignment::Center);
					}
				}
			}

			graph_bounds.Y += (line_height + line_spacing) * (max_depth + 2);
		}
	}
}

void UIScene_Profiler::Draw_Stats(PrimitiveRenderer& pr, MarkupFontRenderer& tr, std::string path, int depth, Rect2D& bounds)
{
	/*
	Statistic** stats;
	int stats_count = StatisticManager::Get_Statistics(stats);

	// Add groups.
	std::vector<std::string> groups;
	for (int i = 0; i < stats_count; i++)
	{
		Statistic& stat = *stats[i];
		if (!stat.Display)
		{
			continue;
		}

		if ((int)stat.Path_Count > depth + 1)
		{
			std::string sub_path = "";
			for (int j = 0; j < depth; j++)
			{
				if (j > 0)
				{
					sub_path += "/";
				}
				sub_path += stat.Path[j];
			}

			if (sub_path == path)
			{
				std::string group = stat.Path[depth];
				if (std::find(groups.begin(), groups.end(), group) == groups.end())
				{
					groups.push_back(group);

					pr.Draw_Solid_Quad(bounds, Color(0.0f, 0.0f, 0.0f, 200.0f));
					tr.Draw_String(group.c_str(), bounds, line_height, Color(255.0f, 0.0f, 0.0f, 255.0f));
					bounds.Y += line_height + line_spacing;

					Draw_Stats(pr, tr, path == "" ? group : path + "/" + group, depth + 1, bounds);
					bounds.Y += (line_spacing * 6);
				}
			}
		}
	}

	// Add stats.
	for (int i = 0; i < stats_count; i++)
	{
		Statistic& stat = *stats[i];
		if (!stat.Display)
		{
			continue;
		}


		if ((int)stat.Path_Count > depth + 1)
		{
			std::string sub_path = "";
			for (int j = 0; j < depth; j++)
			{
				if (j > 0)
				{
					sub_path += "/";
				}
				sub_path += stat.Path[j];
			}

			if (sub_path == path)
			{ 
				std::string name = stat.Path[depth];

				pr.Draw_Solid_Quad(bounds, Color(0.0f, 0.0f, 0.0f, 200.0f));
				tr.Draw_String(name.c_str(), bounds, line_height, Color(0.0f, 255.0f, 0.0f, 255.0f));
				tr.Draw_String(stat.To_String().c_str(), Rect2D(bounds.X + name_width, bounds.Y, bounds.Width, bounds.Height), line_height, Color(0.0f, 255.0f, 0.0f, 255.0f));
				bounds.Y += line_height + line_spacing;
			}
		}
	}
	*/
}
void UIScene_Profiler::Recieve_Event(UIManager* manager, UIEvent e)
{
	/*
	switch (e.Type)
	{
	case UIEventType::ToolbarItem_Click:
		{
			if (e.Source->Get_Name() == "memory_toolbar_item")
				m_page = 0;
			else if (e.Source->Get_Name() == "cpu_toolbar_item")
				m_page = 1;
			else if (e.Source->Get_Name() == "network_toolbar_item")
				m_page = 2;
			
			Refresh_State();

			break;
		}
	}
	*/
}
