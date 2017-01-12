// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_SelectProfile.h"
#include "Game/UI/Scenes/UIScene_Inventory.h"
#include "Game/UI/Scenes/UIScene_ConfirmDialog.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UIFadeTopLevelTransition.h"
#include "Game/UI/Scenes/Helper/UICharacterPreviewRenderer.h"
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
#include "Engine/UI/Elements/UIDropDownMenu.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineClient.h"
#include "Engine/Online/OnlineUser.h"

#include "Engine/Localise/Locale.h"

#include "Engine/Engine/GameEngine.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

#include "Game/Profile/ProfileManager.h"

UIScene_SelectProfile::UIScene_SelectProfile()
	: m_delete_confirm_dialog_open(false)
	, m_delete_profile_index(-1)
	, m_inventory_open(false)
	, m_selected_profile_index(-1)
{
	Set_Layout("select_profile");	

	m_draw_item_delegate = new Delegate<UIScene_SelectProfile, UIListViewDrawItemData>(this, &UIScene_SelectProfile::On_Draw_Item);

	UIListView* listview = Find_Element<UIListView*>("profile_listview");
	listview->On_Draw_Item += m_draw_item_delegate;

	m_char_renderer = new UICharacterPreviewRenderer();
}

UIScene_SelectProfile::~UIScene_SelectProfile()
{
	SAFE_DELETE(m_char_renderer);
	SAFE_DELETE(m_draw_item_delegate);
}

const char* UIScene_SelectProfile::Get_Name()
{
	return "UIScene_SelectProfile";
}

int UIScene_SelectProfile::Get_Selected_Index()
{
	return m_selected_profile_index;
}

bool UIScene_SelectProfile::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_SelectProfile::Should_Display_Cursor()
{
	return true;
}

bool UIScene_SelectProfile::Is_Focusable()
{
	return true;
}

void UIScene_SelectProfile::Enter(UIManager* manager)
{
	Refresh_Items();
}	

void UIScene_SelectProfile::Refresh_Items()
{
	ProfileManager* profile_manager = ProfileManager::Get();
	
	std::vector<OnlineUser*> local_users = OnlinePlatform::Get()->Get_Local_Users();

	UIListView* listview = Find_Element<UIListView*>("profile_listview");
	listview->Clear_Items();
	
	if (ProfileManager::Get()->Get_Profile_Count() < ProfileManager::Get_Max_Profiles())
	{
		listview->Add_Item("", NULL);
	}

	for (int i = 0; i < profile_manager->Get_Profile_Count(); i++)
	{
		bool add_profile = true;

		// Dont show already logged in user.
		for (std::vector<OnlineUser*>::iterator iter = local_users.begin(); iter != local_users.end(); iter++)
		{
			OnlineUser* user = *iter;
			if (user->Get_Input_Source().Device != InputSourceDevice::NONE &&
				user->Get_Profile_Index() == i)
			{
				add_profile = false;
				break;
			}
		}

		if (add_profile == true)
		{
			Profile* profile = profile_manager->Get_Profile(i);
			listview->Add_Item("", reinterpret_cast<void*>(i));
		}
	}
}

void UIScene_SelectProfile::Cancel()
{
	m_selected_profile_index = -1;
	m_inventory_open = false;
}

void UIScene_SelectProfile::Exit(UIManager* manager)
{
}	

void UIScene_SelectProfile::Refresh(UIManager* manager)
{
	m_atlas					= manager->Get_Atlas();
	m_atlas_renderer		= AtlasRenderer(m_atlas);

	m_font					= manager->Get_Font();
	m_font_renderer			= FontRenderer(m_font, false, false);

	m_item_frame_active = UIFrame("screen_main_item_border_active_#");
	m_item_frame_inactive = UIFrame("screen_main_item_border_inactive_#");

	UIScene::Refresh(manager);
}

void UIScene_SelectProfile::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	if (m_inventory_open && manager->Get_Top_Scene_Index() == scene_index)
	{
		manager->Get_Scene_From_Top(1)->Dispatch_Event(manager, UIEvent(UIEventType::Dialog_Close, NULL, this));	
		manager->Go(UIAction::Pop(new UIFadeOutTransition()));
		return;
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_SelectProfile::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_SelectProfile::On_Draw_Item(UIListViewDrawItemData* data)
{
	PrimitiveRenderer prim_renderer;

	bool can_create = (ProfileManager::Get()->Get_Profile_Count() < ProfileManager::Get_Max_Profiles());

	// Draw outline.
	if (data->selected)
		m_item_frame_active.Draw_Frame(m_atlas_renderer, data->item_bounds, data->ui_scale, Color::White);
	else if (data->hovering)
		m_item_frame_active.Draw_Frame(m_atlas_renderer, data->item_bounds, data->ui_scale, Color(128, 128, 128, 255));
	else
		m_item_frame_inactive.Draw_Frame(m_atlas_renderer, data->item_bounds, data->ui_scale, Color::White);

	// Create new-char or profile.
	if (data->item_index == 0 && can_create == true)
	{
		// Draw "New Character Text"
		m_font_renderer.Draw_String(
			S("#menu_select_character_new_character"), 
			Rect2D(
				data->item_bounds.X, 
				data->item_bounds.Y + (data->item_bounds.Height * 0.20f) , 
				data->item_bounds.Width, 
				data->item_bounds.Height * 0.5f
			),  
			16.0f,
			Color(180, 180, 180, 255), 
			TextAlignment::Center,
			TextAlignment::Center,
			data->ui_scale * 0.5f);
	}
	else
	{
		UIListView* profile_list = Find_Element<UIListView*>("profile_listview");
		Profile* profile = ProfileManager::Get()->Get_Profile(reinterpret_cast<int>(data->item->MetaData));

		const float spacing = 2.0f * data->ui_scale.Y;
		const float text_height = 9.0f * data->ui_scale.Y;

		Rect2D avatar_bounds = Rect2D(
			data->item_bounds.X + (spacing),
			data->item_bounds.Y + (spacing),
			data->item_bounds.Height - (spacing * 2.0f),
			data->item_bounds.Height - (spacing * 2.0f)
		);
		Rect2D name_bounds = Rect2D(
			avatar_bounds.X + avatar_bounds.Width + (spacing * 2),
			avatar_bounds.Y,
			data->item_bounds.Width - (spacing * 3) - data->item_bounds.Height,
			text_height
		);
		Rect2D level_bounds = Rect2D(
			name_bounds.X,
			name_bounds.Y + name_bounds.Height + spacing,
			name_bounds.Width,
			name_bounds.Height
		);
		Rect2D prog_bounds = Rect2D(
			name_bounds.X - spacing,
			level_bounds.Y + level_bounds.Height + spacing,
			name_bounds.Width,
			name_bounds.Height
		);
		Rect2D prog_bar_rect = Rect2D(
			prog_bounds.X + spacing,
			prog_bounds.Y + spacing, 
			prog_bounds.Width - (spacing * 2), 
			prog_bounds.Height - (spacing * 2)
		);

		// Draw character.
		m_item_frame_inactive.Draw_Frame(m_atlas_renderer, avatar_bounds, data->ui_scale, Color(200, 200, 200, 255));

		// Draw character.
		m_char_renderer->Draw_Head(data->time, data->manager, profile, avatar_bounds);

		// Draw name.
		m_font_renderer.Draw_String(
			profile->Name.c_str(), 
			name_bounds,
			16.0f,
			Color(200, 200, 200, 255), 
			TextAlignment::Left,
			TextAlignment::Top,
			data->ui_scale * 0.5f);

		// Draw level.
		m_font_renderer.Draw_String(
			SF("#menu_select_character_level", profile->Level), 
			level_bounds,
			16.0f,
			Color(200, 200, 200, 255), 
			TextAlignment::Left,
			TextAlignment::Top,
			data->ui_scale * 0.4f);

		// Draw XP bar.
		m_item_frame_inactive.Draw_Frame(m_atlas_renderer, prog_bounds, data->ui_scale * 0.5f);

		// Draw level progress.
		float progress = profile->Get_Level_Progress();
		if (profile->Level == ProfileManager::Get()->Get_Level_Cap())
		{
			progress = 1.0f;
		}

		Color bg_color = Color(32, 32, 32, 255);
		Color color = Color(255, 194, 14, 255);
		prim_renderer.Draw_Solid_Quad(prog_bar_rect, bg_color);
		prim_renderer.Draw_Solid_Quad(Rect2D(prog_bar_rect.X, prog_bar_rect.Y, (prog_bar_rect.Width * progress), prog_bar_rect.Height), color);
	}
}

void UIScene_SelectProfile::Create_Profile()
{
	DBG_LOG("Creating new profile ...");
	UIManager* manager = GameEngine::Get()->Get_UIManager();
	int profile_index = ProfileManager::Get()->Create_Profile();

	// Save profile data.
	Game::Get()->Queue_Save();

	m_selected_profile_index = profile_index;
	m_inventory_open = true;
	manager->Go(UIAction::Push(new UIScene_Inventory(ProfileManager::Get()->Get_Profile(profile_index), true, profile_index), new UISlideInTransition()));	
}

void UIScene_SelectProfile::Select_Profile(int index)
{
	UIManager* manager = GameEngine::Get()->Get_UIManager();

	DBG_LOG("Selecting profile %i ...", index);
	
	m_selected_profile_index = index;
	manager->Get_Scene_From_Top(1)->Dispatch_Event(manager, UIEvent(UIEventType::Dialog_Close, NULL, this));	
	manager->Go(UIAction::Pop(new UIFadeOutTransition()));

	manager->Play_UI_Sound(UISoundType::Close_Popup);
}

void UIScene_SelectProfile::Delete_Profile(int index)
{
	DBG_LOG("Deleting profile %i ...", index);

	ProfileManager::Get()->Delete_Profile(index);

	std::vector<OnlineUser*> local_users = OnlinePlatform::Get()->Get_Local_Users();

	// Adjust indexes of other user profiles.
	for (std::vector<OnlineUser*>::iterator iter = local_users.begin(); iter != local_users.end(); iter++)
	{
		OnlineUser* user = *iter;
		if (user->Get_Profile_Index() >= index)
		{
			user->Set_Profile_Index(user->Get_Profile_Index() - 1);
		}
	}

	// Save profile data.
	Game::Get()->Queue_Save();

	Refresh_Items();
}

void UIScene_SelectProfile::Recieve_Event(UIManager* manager, UIEvent e)
{	
	switch (e.Type)
	{
	case UIEventType::Dialog_Close:
		{
			if (m_delete_confirm_dialog_open == true)
			{
				if (dynamic_cast<UIScene_ConfirmDialog*>(e.Scene)->Get_Selected_Index() == 1)
				{
					Delete_Profile(m_delete_profile_index);
				}
				m_delete_confirm_dialog_open = false;
			}
			break;
		}
	case UIEventType::ListView_Item_Click:
		{
			if (e.Source->Get_Name() == "profile_listview")
			{
				UIListView* view = dynamic_cast<UIListView*>(e.Source);
				int index = view->Get_Selected_Item_Index();
				
				bool can_create = (ProfileManager::Get()->Get_Profile_Count() < ProfileManager::Get_Max_Profiles());

				if (index == 0 && can_create)
				{
					Create_Profile();
				}
				else
				{
					UIDropDownMenu* drop_down_menu = Find_Element<UIDropDownMenu*>("profile_option_menu");
					UIListView* profile_list = Find_Element<UIListView*>("profile_listview");

					Rect2D selection_box = profile_list->Get_Selected_Item_Rectangle();

					drop_down_menu->Show(Rect2D(selection_box.X, selection_box.Y + selection_box.Height, selection_box.Width, drop_down_menu->Get_Screen_Box().Height));
				}
			}
			break;
		}
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "back_button")
			{
				m_selected_profile_index = -1;
				manager->Get_Scene_From_Top(1)->Dispatch_Event(manager, UIEvent(UIEventType::Dialog_Close, NULL, this));	

				manager->Go(UIAction::Pop(new UIFadeOutTransition()));
			}
			else if (e.Source->Get_Name() == "use_button")
			{
				UIDropDownMenu* drop_down_menu = Find_Element<UIDropDownMenu*>("profile_option_menu");
				UIListView* profile_list = Find_Element<UIListView*>("profile_listview");

				drop_down_menu->Hide();
				
				bool can_create = (ProfileManager::Get()->Get_Profile_Count() < ProfileManager::Get_Max_Profiles());

				Select_Profile(reinterpret_cast<int>(profile_list->Get_Selected_Item().MetaData));
			}
			else if (e.Source->Get_Name() == "delete_button")
			{
				UIDropDownMenu* drop_down_menu = Find_Element<UIDropDownMenu*>("profile_option_menu");
				UIListView* profile_list = Find_Element<UIListView*>("profile_listview");

				drop_down_menu->Hide();
				
				manager->Go(UIAction::Push(new UIScene_ConfirmDialog(S("#menu_select_character_confirm_delete"), S("#menu_no"), S("#menu_yes")), new UIFadeInTransition()));
			//	manager->Play_UI_Sound(UISoundType::Popup);

				m_delete_confirm_dialog_open = true;				
				
				bool can_create = (ProfileManager::Get()->Get_Profile_Count() < ProfileManager::Get_Max_Profiles());
				
				m_delete_profile_index = reinterpret_cast<int>(profile_list->Get_Selected_Item().MetaData);
			}
			else if (e.Source->Get_Name() == "close_button")
			{
				UIDropDownMenu* drop_down_menu = Find_Element<UIDropDownMenu*>("profile_option_menu");
				UIListView* profile_list = Find_Element<UIListView*>("profile_listview");

				drop_down_menu->Hide();
			}
			break;
		}
	}
}
