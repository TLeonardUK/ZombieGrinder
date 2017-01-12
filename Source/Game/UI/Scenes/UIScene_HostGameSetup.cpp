// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_HostGameSetup.h"
#include "Game/UI/Scenes/UIScene_HostGame.h"
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
#include "Engine/UI/Elements/UICheckBox.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UIListView.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineClient.h"

#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Engine/Localise/Locale.h"

#include "Generic/Math/Math.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_HostGameSetup::UIScene_HostGameSetup(bool change_lobby_settings)
	: m_changing_lobby_settings(change_lobby_settings)
{
	Set_Layout("host_game_setup");	
}

const char* UIScene_HostGameSetup::Get_Name()
{
	return "UIScene_HostGameSetup";
}

bool UIScene_HostGameSetup::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_HostGameSetup::Should_Display_Cursor()
{
	return true;
}

bool UIScene_HostGameSetup::Is_Focusable()
{
	return true;
}

void UIScene_HostGameSetup::Enter(UIManager* manager)
{
}	

void UIScene_HostGameSetup::Exit(UIManager* manager)
{
}	

void UIScene_HostGameSetup::Refresh(UIManager* manager)
{
	UIScene::Refresh(manager);
	
	OnlineMatching_LobbySettings settings = OnlineMatching::Get()->Get_Lobby_Settings();

	UIComboBox* map_box = Find_Element<UIComboBox*>("map_box");
	UIComboBox* game_mode_box = Find_Element<UIComboBox*>("game_mode_box");
	UISlider* count_slider = Find_Element<UISlider*>("player_count_slider");
	UICheckBox* secure_checkbox = Find_Element<UICheckBox*>("secure_checkbox");
	UICheckBox* private_checkbox = Find_Element<UICheckBox*>("private_checkbox");
	UICheckBox* local_checkbox = Find_Element<UICheckBox*>("local_checkbox");

	std::vector<std::string> existing_game_modes;

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
				game_mode_box->Set_Selected_Item_Index((int)game_mode_box->Get_Items().size() - 1);
			}
		}
	}

	// Refresh map list.
	Refresh_Map_List();

	// Apply default settings.
	if (m_changing_lobby_settings == true)
	{
		MapFileHandle* handle = ResourceFactory::Get()->Get_Map_File_By_GUID(settings.MapGUID.c_str());
		DBG_ASSERT(handle != NULL);
		
		int min_players = Max(handle->Get()->Get_Header()->Min_Players, OnlinePlatform::Get()->Get_Local_User_Count());
		int max_players = handle->Get()->Get_Header()->Max_Players;
		int range = max_players - min_players;
		float progress = 1.0f;

		if (range >= 1)
		{
			progress = (float)(settings.MaxPlayers - min_players) / (float)range;
		}

		map_box->Select_Item_By_MetaData(handle);

		count_slider->Set_Progress(progress);
		secure_checkbox->Set_Checked(settings.Secure);
		local_checkbox->Set_Checked(settings.Local);
		private_checkbox->Set_Checked(settings.Private);
	}
	else
	{
		secure_checkbox->Set_Checked(true);
		local_checkbox->Set_Checked(true);
		private_checkbox->Set_Checked(false);
		count_slider->Set_Progress(0.5f);	
	}

	Refresh_Preview();
	Refresh_Player_Count();
}

void UIScene_HostGameSetup::Refresh_Map_List()
{
	UIComboBox* map_box = Find_Element<UIComboBox*>("map_box");
	UIComboBox* game_mode_box = Find_Element<UIComboBox*>("game_mode_box");

	std::string current_game_mode = game_mode_box->Get_Selected_Item().Text;

	map_box->Clear_Items();

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

void UIScene_HostGameSetup::Refresh_Preview()
{
	UIComboBox* map_box = Find_Element<UIComboBox*>("map_box");
	UIPanel* map_panel = Find_Element<UIPanel*>("map_image_panel");
	UILabel* name_label = Find_Element<UILabel*>("map_name_label");
	UILabel* description_label = Find_Element<UILabel*>("map_description_label");

	MapFileHandle* handle = static_cast<MapFileHandle*>(map_box->Get_Selected_Item().MetaData1);

	name_label->Set_Value(S(handle->Get()->Get_Header()->Long_Name.c_str()));
	description_label->Set_Value(StringHelper::Replace(S(handle->Get()->Get_Header()->Description.c_str()), "\\n", "\n"));
	map_panel->Set_Background_Image(handle->Get()->Get_Preview_Image());
}

void UIScene_HostGameSetup::Refresh_Player_Count()
{
	UIComboBox* map_box = Find_Element<UIComboBox*>("map_box");
	UILabel* count_label = Find_Element<UILabel*>("player_count_box");
	UISlider* count_slider = Find_Element<UISlider*>("player_count_slider");
	
	// Update player count.	
	MapFileHandle* handle = static_cast<MapFileHandle*>(map_box->Get_Selected_Item().MetaData1);
	int min_players = Max(handle->Get()->Get_Header()->Min_Players, OnlinePlatform::Get()->Get_Local_User_Count());
	int max_players = handle->Get()->Get_Header()->Max_Players;
	int players     = (int)(min_players + ((max_players - min_players) * count_slider->Get_Progress()));
	count_label->Set_Value(StringHelper::To_String(players));
}

void UIScene_HostGameSetup::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	Refresh_Player_Count();
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_HostGameSetup::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_HostGameSetup::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::ComboBox_SelectedIndexChanged:
		{
			if (e.Source->Get_Name() == "game_mode_box")
			{
				Refresh_Map_List();
				Refresh_Preview();
				Refresh_Player_Count();
			}
			else if (e.Source->Get_Name() == "map_box")
			{
				Refresh_Preview();
				Refresh_Player_Count();
			}
		}
		break;
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "ok_button")
			{
				UICheckBox* private_checkbox = Find_Element<UICheckBox*>("private_checkbox");
				UICheckBox* secure_checkbox = Find_Element<UICheckBox*>("secure_checkbox");
				UICheckBox* local_checkbox = Find_Element<UICheckBox*>("local_checkbox");
				UIComboBox* map_box = Find_Element<UIComboBox*>("map_box");
				UISlider* count_slider = Find_Element<UISlider*>("player_count_slider");

				MapFileHandle* map_file = static_cast<MapFileHandle*>(map_box->Get_Selected_Item().MetaData1);
				int min_players = Max(map_file->Get()->Get_Header()->Min_Players, OnlinePlatform::Get()->Get_Local_User_Count());
				int max_players = map_file->Get()->Get_Header()->Max_Players;
				int players     = (int)(min_players + ((max_players - min_players) * count_slider->Get_Progress()));

				if (m_changing_lobby_settings == true)
				{
					OnlineMatching_LobbySettings settings = OnlineMatching::Get()->Get_Lobby_Settings();
					settings.MapGUID		= map_file->Get()->Get_Header()->GUID;
					settings.MapWorkshopID	= map_file->Get()->Get_Header()->Workshop_ID;
					settings.MaxPlayers		= players;
					settings.Private		= private_checkbox->Get_Checked();
					settings.Secure			= secure_checkbox->Get_Checked();
					settings.Local			= local_checkbox->Get_Checked();
					OnlineMatching::Get()->Set_Lobby_Settings(settings);

					manager->Go(UIAction::Pop(new UIFadeOutTransition()));
				}
				else
				{
					manager->Go(UIAction::Replace(new UIScene_HostGame(map_file, players, private_checkbox->Get_Checked(), secure_checkbox->Get_Checked(), local_checkbox->Get_Checked()), new UIFadeTopLevelTransition()));
				}
			}
			else if (e.Source->Get_Name() == "cancel_button")
			{
				manager->Go(UIAction::Pop(new UIFadeTopLevelTransition()));
			}
		}
		break;
	}
}
