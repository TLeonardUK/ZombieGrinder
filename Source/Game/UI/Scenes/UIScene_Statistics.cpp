// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Statistics.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UISlideInTopLevelTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UICheckBox.h"
#include "Engine/UI/Elements/UIComboBox.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Online/StatisticsManager.h"
#include "Engine/Online/OnlineAchievements.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

bool Statistic_Category_Sort(Statistic* a, Statistic* b)
{
	return strcmp(b->category.c_str(), a->category.c_str()) < 0;
}

UIScene_Statistics::UIScene_Statistics()
{
	Set_Layout("statistics");

	UIListView* listview = Find_Element<UIListView*>("statistic_listview");
	listview->Clear_Columns();
	listview->Add_Column(S("#menu_statistics_name_column"), 0.70f, false, true);
	listview->Add_Column(S("#menu_statistics_value_column"), 0.30f, false, true);

	UIComboBox* sourcebox = Find_Element<UIComboBox*>("source_box");
	sourcebox->Add_Item(S("#menu_statistics_local"));
	sourcebox->Add_Item(S("#menu_statistics_global"));
}

const char* UIScene_Statistics::Get_Name()
{
	return "UIScene_Statistics";
}

bool UIScene_Statistics::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_Statistics::Should_Display_Cursor()
{
	return true;
}

bool UIScene_Statistics::Is_Focusable()
{
	return true;
}

void UIScene_Statistics::Refresh_Items()
{	
	StatisticsManager* stat_manager = StatisticsManager::Get();
	
	UIListView* listview = Find_Element<UIListView*>("statistic_listview");
	UIComboBox* sourcebox = Find_Element<UIComboBox*>("source_box");
	listview->Clear_Items();

	std::vector<Statistic*> stats = stat_manager->Get_Stats();
	std::sort(stats.begin(), stats.end(), Statistic_Category_Sort);
	
	std::string last_category = "";
	std::vector<std::string> values;

	bool display_global = (sourcebox->Get_Selected_Item_Index() == 1);

	for (std::vector<Statistic*>::iterator iter = stats.begin(); iter != stats.end(); iter++)
	{
		Statistic* current_stat = *iter;
		if (current_stat->display == true && (current_stat->aggregated == display_global))
		{
			if (last_category != current_stat->category)
			{
				// Add category values.
				values.clear();
				values.push_back(StringHelper::Format("[c=0,200,200,255]%s[/c]", S(current_stat->category)));
				values.push_back("");

				if (last_category != "")
				{
					listview->Add_Item("", NULL);
				}
				listview->Add_Item(values, NULL);

				last_category = current_stat->category;
			}

			// Add item values.
			values.clear();
			values.push_back(StringHelper::Format("    %s", S(current_stat->name)));

			if (display_global == true)
			{
				values.push_back(StringHelper::Format_Number(current_stat->aggregated_value));
			}
			else
			{
				values.push_back(StringHelper::Format_Number(current_stat->value));
			}

			listview->Add_Item(values, current_stat);
		}
	}

	// Add weapon stats.
}

void UIScene_Statistics::Enter(UIManager* manager)
{
	OnlineAchievements::Get()->Force_Sync();

	Refresh_Items();
}	

void UIScene_Statistics::Exit(UIManager* manager)
{
}	

void UIScene_Statistics::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Statistics::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_Statistics::Refresh(UIManager* manager)
{
	m_atlas					= manager->Get_Atlas();
	m_atlas_renderer		= AtlasRenderer(m_atlas);

	m_font					= manager->Get_Font();
	m_font_renderer			= FontRenderer(m_font, false, false);

	m_item_frame_active = UIFrame("screen_main_item_border_active_#");
	m_item_frame_inactive = UIFrame("screen_main_item_border_inactive_#");

	UIScene::Refresh(manager);
}

void UIScene_Statistics::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::ComboBox_SelectedIndexChanged:
		{
			if (e.Source->Get_Name() == "source_box")
			{
				Refresh_Items();
			}
			break;
		}
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "back_button")
			{
				manager->Go(UIAction::Pop(new UISlideInTopLevelTransition()));
			}
			break;
		}
	}
}
