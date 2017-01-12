// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_FindGameSetup.h"
#include "Game/UI/Scenes/UIScene_FindGame.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Transitions/UIFadeTopLevelTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIComboBox.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UIListView.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/UI/Elements/UICheckBox.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineClient.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_FindGameSetup::UIScene_FindGameSetup()
{
	Set_Layout("find_game_setup");	
}

const char* UIScene_FindGameSetup::Get_Name()
{
	return "UIScene_FindGameSetup";
}

bool UIScene_FindGameSetup::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_FindGameSetup::Should_Display_Cursor()
{
	return true;
}

bool UIScene_FindGameSetup::Is_Focusable()
{
	return true;
}

void UIScene_FindGameSetup::Enter(UIManager* manager)
{
}	

void UIScene_FindGameSetup::Exit(UIManager* manager)
{
}	

void UIScene_FindGameSetup::Refresh(UIManager* manager)
{
	UIScene::Refresh(manager);

	// Adds maps?
	UIComboBox* map_box = Find_Element<UIComboBox*>("map_box");
	UIComboBox* game_mode_box = Find_Element<UIComboBox*>("game_mode_box");
	UICheckBox* lobbies_checkbox = Find_Element<UICheckBox*>("game_checkbox");
	UICheckBox* games_checkbox = Find_Element<UICheckBox*>("lobby_checkbox");

	lobbies_checkbox->Set_Checked(true);
	games_checkbox->Set_Checked(true);

	std::vector<std::string> existing_game_modes;

	game_mode_box->Add_Item(S("#menu_game_setup_find_game_any_map"), NULL);

	// Add all maps.
	std::vector<MapFileHandle*> files = ResourceFactory::Get()->Get_Sorted_Map_Files();
	for (std::vector<MapFileHandle*>::iterator iter = files.begin(); iter != files.end(); iter++)
	{
		MapFileHandle* handle = *iter;
		MapFile* file = handle->Get();

		// We don't care about internal map files.
		if (file->Is_Internal())
		{
			continue;
		}

		// Map requires less players than local-number.
		if (file->Get_Header()->Max_Players < OnlinePlatform::Get()->Get_Local_User_Count())
		{
			continue;
		}

		// Add game mode.
		std::string game_mode = file->Get_Header()->Short_Name;
		size_t pos = game_mode.find('_');
		if (pos != std::string::npos)
		{
			game_mode = game_mode.substr(0, pos);
		}

		std::string locale_string = StringHelper::Format("#game_mode_prefix_name_%s", game_mode.c_str());
		const char* result = Locale::Get()->Get_String(locale_string.c_str(), true);
		if (result == NULL)
		{
			locale_string = "#game_mode_prefix_name_misc";
		}

		if (std::find(existing_game_modes.begin(), existing_game_modes.end(), locale_string) == existing_game_modes.end())
		{
			game_mode_box->Add_Item(locale_string.c_str(), NULL);
			existing_game_modes.push_back(locale_string);

			// Start on campaign 1.
			if (game_mode == "c1")
			{
			//	game_mode_box->Set_Selected_Item_Index((int)game_mode_box->Get_Items().size() - 1);
			}
		}
	}

	// Refresh map list.
	Refresh_Map_List();

	Refresh_Preview();
}

void UIScene_FindGameSetup::Refresh_Map_List()
{
	UIComboBox* map_box = Find_Element<UIComboBox*>("map_box");
	UIComboBox* game_mode_box = Find_Element<UIComboBox*>("game_mode_box");

	std::string current_game_mode = game_mode_box->Get_Selected_Item().Text;

	map_box->Clear_Items();

	map_box->Set_Enabled(!(game_mode_box->Get_Selected_Item_Index() == 0));

	if (game_mode_box->Get_Selected_Item_Index() == 0)
	{
		return;
	}

	// Add all maps.
	std::vector<MapFileHandle*> files = ResourceFactory::Get()->Get_Sorted_Map_Files();
	for (std::vector<MapFileHandle*>::iterator iter = files.begin(); iter != files.end(); iter++)
	{
		MapFileHandle* handle = *iter;
		MapFile* file = handle->Get();

		// We don't care about internal map files.
		if (file->Is_Internal())
		{
			continue;
		}

		// Map requires less players than local-number.
		if (file->Get_Header()->Max_Players < OnlinePlatform::Get()->Get_Local_User_Count())
		{
			continue;
		}

		// Add game mode.
		std::string game_mode = file->Get_Header()->Short_Name;
		size_t pos = game_mode.find('_');
		if (pos != std::string::npos)
		{
			game_mode = game_mode.substr(0, pos);
		}

		std::string locale_string = StringHelper::Format("#game_mode_prefix_name_%s", game_mode.c_str());
		const char* result = Locale::Get()->Get_String(locale_string.c_str(), true);
		if (result == NULL)
		{
			locale_string = "#game_mode_prefix_name_misc";
		}

		if (locale_string == current_game_mode)
		{
			std::string long_name = file->Get_Header()->Long_Name;
			pos = long_name.find('-');
			if (pos != std::string::npos)
			{
				long_name = StringHelper::Trim(long_name.substr(pos + 1).c_str());
			}

			map_box->Add_Item(long_name.c_str(), handle);
		}
	}
}

void UIScene_FindGameSetup::Refresh_Preview()
{
	UIComboBox* map_box = Find_Element<UIComboBox*>("map_box");
	UIPanel* map_panel = Find_Element<UIPanel*>("map_image_panel");
	UILabel* name_label = Find_Element<UILabel*>("map_name_label");
	UILabel* description_label = Find_Element<UILabel*>("map_description_label");
	UIComboBox* game_mode_box = Find_Element<UIComboBox*>("game_mode_box");

	if (game_mode_box->Get_Selected_Item_Index() == 0)
	{
		name_label->Set_Value("");
		description_label->Set_Value("");
		map_panel->Set_Background_Image(ResourceFactory::Get()->Get_Atlas_Frame("screen_main_map_preview"));
	}
	else
	{
		MapFileHandle* handle = static_cast<MapFileHandle*>(map_box->Get_Selected_Item().MetaData1);

		name_label->Set_Value(S(handle->Get()->Get_Header()->Long_Name.c_str()));
		description_label->Set_Value(StringHelper::Replace(S(handle->Get()->Get_Header()->Description.c_str()), "\\n", "\n"));
		map_panel->Set_Background_Image(handle->Get()->Get_Preview_Image());
	}
}

void UIScene_FindGameSetup::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_FindGameSetup::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_FindGameSetup::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::ComboBox_SelectedIndexChanged:
		{
			if (e.Source->Get_Name() == "game_mode_box")
			{
				Refresh_Map_List();
				Refresh_Preview();
			}
			else if (e.Source->Get_Name() == "map_box")
			{
				Refresh_Preview();
			}
		}
		break;
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "ok_button")
			{
				UIComboBox* map_box = Find_Element<UIComboBox*>("map_box");
				UICheckBox* lobbies_checkbox = Find_Element<UICheckBox*>("game_checkbox");
				UICheckBox* games_checkbox = Find_Element<UICheckBox*>("lobby_checkbox");
				UIComboBox* game_mode_box = Find_Element<UIComboBox*>("game_mode_box");
				MapFileHandle* map_file = (game_mode_box->Get_Selected_Item_Index() == 0) ? NULL : static_cast<MapFileHandle*>(map_box->Get_Selected_Item().MetaData1);

				manager->Go(UIAction::Replace(new UIScene_FindGame(map_file, lobbies_checkbox->Get_Checked(), games_checkbox->Get_Checked()), new UIFadeTopLevelTransition()));
			}
			else if (e.Source->Get_Name() == "cancel_button")
			{
				manager->Go(UIAction::Pop(new UIFadeTopLevelTransition()));
			}
		}
		break;
	}
}
