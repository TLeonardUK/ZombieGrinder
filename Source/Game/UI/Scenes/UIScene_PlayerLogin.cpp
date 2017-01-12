// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_PlayerLogin.h"
#include "Game/UI/Scenes/UIScene_SelectProfile.h"
#include "Game/UI/Scenes/UIScene_MainMenu.h"
#include "Game/UI/Scenes/UIScene_FindGame.h"
#include "Game/UI/Scenes/UIScene_HostGame.h"
#include "Game/UI/Scenes/UIScene_GameSetupMenu.h"
#include "Game/UI/Scenes/Helper/UICharacterPreviewRenderer.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/UI/Elements/UIButton.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Game/Runner/GameOptions.h"
#include "Engine/Engine/EngineOptions.h"

#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineUser.h"
#include "Engine/Online/OnlineClient.h"

#include "Engine/Engine/GameEngine.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Profile/ProfileManager.h"
#include "Game/Profile/Profile.h"

#include "Game/Runner/Game.h"

UIScene_PlayerLogin::UIScene_PlayerLogin(bool accept_invite)
	: m_accept_invite(accept_invite)
	, m_blink(false)
	, m_blink_timer(0.0f)
	, m_select_profile_index(-1)
	, m_select_profile_source(InputSourceDevice::Keyboard, 0)
	, m_selected_profile_index(-1)
{
	Set_Layout("player_login");	

	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		m_login_indexes[i] = -1;
	}

	m_char_renderer = new UICharacterPreviewRenderer();
}

UIScene_PlayerLogin::~UIScene_PlayerLogin()
{
	SAFE_DELETE(m_char_renderer);
}

bool UIScene_PlayerLogin::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_PlayerLogin::Get_Name()
{
	return "UIScene_PlayerLogin";
}

bool UIScene_PlayerLogin::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_PlayerLogin::Should_Display_Cursor()
{
	return true;
}

bool UIScene_PlayerLogin::Is_Focusable()
{
	return true;
}

UIScene* UIScene_PlayerLogin::Get_Background(UIManager* manager)
{
	return manager->Retrieve_Persistent_Scene("UIScene_MenuBackgroundSimple");
}	

bool UIScene_PlayerLogin::Player_Logged_In(int index)
{
	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		if (m_login_indexes[i] == index)
		{
			return true;
		}
	}

	return false;
}

bool UIScene_PlayerLogin::Any_Player_Logged_In()
{
	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		if (m_login_indexes[i] != -1)
		{
			return true;
		}
	}

	return false;
}

void UIScene_PlayerLogin::Login_Player_Index(int index, InputSource source, int profile_index)
{
#ifdef OPT_PREMIUM_ACCOUNTS
	bool is_premium = OnlinePlatform::Get()->Check_Purchased(*GameOptions::premium_account_appid);
#endif

	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		if (i == index)
		{
			if (index > 0)
			{
				OnlineUser* user = OnlinePlatform::Get()->Login_Local_User(index);
				user->Set_Input_Source(source);
				user->Set_Profile_Index(profile_index);
#ifdef OPT_PREMIUM_ACCOUNTS
				user->Set_Premium(is_premium);
#endif
			}
			else
			{
				OnlineUser* user = OnlinePlatform::Get()->Get_Local_User_By_Index(index);
				user->Set_Input_Source(source);
				user->Set_Profile_Index(profile_index);
#ifdef OPT_PREMIUM_ACCOUNTS
				user->Set_Premium(is_premium);
#endif
			}

			m_login_indexes[i] = index;
			break;
		}
	}

	Refresh_Screen();
}

int UIScene_PlayerLogin::Input_Source_To_Player_Index(InputSource source)
{
	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		if (m_login_indexes[i] != -1)
		{
			OnlineUser* user = OnlinePlatform::Get()->Get_Local_User_By_Index(m_login_indexes[i]);
			if (user->Get_Input_Source() == source)
			{
				return i;
			}
		}
	}

	return -1;
}

int UIScene_PlayerLogin::Get_Free_Player_Index()
{
	for (int i = 0; MAX_LOCAL_PLAYERS; i++)
	{
		if (m_login_indexes[i] == -1)
		{
			return i;
		}
	}

	return -1;
}

void UIScene_PlayerLogin::Logout_Player_Index(int index)
{
	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		if (m_login_indexes[i] == index)
		{
			if (index > 0)
			{
				OnlinePlatform::Get()->Logout_Local_User(index);
			}
			else
			{
				OnlineUser* user = OnlinePlatform::Get()->Get_Local_User_By_Index(index);
				user->Set_Input_Source(InputSource(InputSourceDevice::NONE, -1));
			}

			m_login_indexes[i] = -1;
			break;
		}
	}

	Align_Logins_Array();

	Refresh_Screen();
}

void UIScene_PlayerLogin::Swap_Login(int from, int to)
{
	DBG_LOG("Swapped login %i to %i", from, to);

	// Swap index in the array.
	m_login_indexes[to] = m_login_indexes[from];
	m_login_indexes[from] = -1;

	// If we are copying to index 0, change the initial-user info to 
	// the shifted device-index.
	
	if (to == 0)
	{
		OnlineUser* to_user = OnlinePlatform::Get()->Get_Local_User_By_Index(to);
		OnlineUser* from_user = OnlinePlatform::Get()->Get_Local_User_By_Index(from);
		
		to_user->Set_Input_Source(from_user->Get_Input_Source());
		to_user->Set_Profile_Index(from_user->Get_Profile_Index());
		m_login_indexes[to] = to;
	}
	
	// Otherwise, just change the local player index.
	else
	{
		OnlineUser* from_user = OnlinePlatform::Get()->Get_Local_User_By_Index(from);
		
		from_user->Set_Local_Player_Index(to);
		m_login_indexes[to] = to;
	}
}

void UIScene_PlayerLogin::Align_Logins_Array()
{
	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		if (m_login_indexes[i] == -1)
		{
			for (int j = i; j < MAX_LOCAL_PLAYERS; j++)
			{
				if (m_login_indexes[j] != -1)
				{
					Swap_Login(j, i);
					break;
				}
			}
		}
	}
}

void UIScene_PlayerLogin::Refresh_Screen()
{
	// Has keyboard been taken?
	bool keyboard_in_use = false;
	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		if (m_login_indexes[i] != -1)
		{
			OnlineUser* user = OnlinePlatform::Get()->Get_Local_User_By_Index(m_login_indexes[i]);
			if (user->Get_Input_Source().Device == InputSourceDevice::Keyboard)
			{
				keyboard_in_use = true;
				break;
			}
		}
	}

	// Update player name labels.
	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		std::string name_label_name = StringHelper::Format("player_%i_name_label", i + 1);
		std::string start_label_name = StringHelper::Format("player_%i_start_label", i + 1);
		UILabel* label_element = Find_Element<UILabel*>(name_label_name.c_str());
		UILabel* start_element = Find_Element<UILabel*>(start_label_name.c_str());
			
		if (m_login_indexes[i] >= 0)
		{
			OnlineUser* user = OnlinePlatform::Get()->Get_Local_User_By_Index(m_login_indexes[i]);
			InputSource source = user->Get_Input_Source();

			label_element->Set_Value(user->Get_Username().c_str());

			if (source.Device == InputSourceDevice::Keyboard)
			{
				start_element->Set_Value(S("#menu_player_login_keyboard"));
			}
			else
			{
				start_element->Set_Value(SF("#menu_player_login_joystick", source.Device_Index + 1));	
			}
		}
		else
		{
			label_element->Set_Value("");

			if (m_blink == true)
			{
				start_element->Set_Value("");
			}
			else
			{
				if (keyboard_in_use == true)
				{
					start_element->Set_Value(S("#menu_player_login_press_to_join"));
				}
				else
				{
					start_element->Set_Value(S("#menu_player_login_press_to_join_first_player"));
				}
			}
		}
	}
}

void UIScene_PlayerLogin::Enter(UIManager* manager)
{
	// Logout everyone.
	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		m_login_indexes[i] = -1;
	}

	// Login all current users.
	std::vector<OnlineUser*> local_users = OnlinePlatform::Get()->Get_Local_Users();
	for (std::vector<OnlineUser*>::iterator iter = local_users.begin(); iter != local_users.end(); iter++)
	{
		OnlineUser* user = *iter;
		if (user->Get_Input_Source().Device != InputSourceDevice::NONE)
		{
			Login_Player_Index(user->Get_Local_Player_Index(), user->Get_Input_Source(), user->Get_Profile_Index());
		}
	}
}	

void UIScene_PlayerLogin::Exit(UIManager* manager)
{
	// Make sure all users are created in online platform.
}	

void UIScene_PlayerLogin::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	Input* main_input = Input::Get();
	
	// Auto load map.
	if (*GameOptions::editor == true || 
		*GameOptions::map != "" ||
		*EngineOptions::connect != 0)
	{
		DBG_ASSERT_STR(ProfileManager::Get()->Get_Profile_Count() > 0, "There must be profiles available to auto-login.");

		int profile_id = 0;
		if (*EngineOptions::client_id > 0)
		{
			profile_id = (*EngineOptions::client_id) % ProfileManager::Get()->Get_Profile_Count();
		}

		Login_Player_Index(0, InputSource(InputSourceDevice::Keyboard, 0), profile_id);

		std::vector<int> used_sources;

		for (int i = 1; i < *GameOptions::local_players; i++)
		{
			int free_joystick = 0;

			for (int j = 0; j < MAX_INPUT_SOURCES; j++)
			{
				if (std::find(used_sources.begin(), used_sources.end(), j) != used_sources.end())
				{
					continue;
				}

				if (Input::Get(j)->Get_Joystick_State()->Is_Connected())
				{
					free_joystick = j;
					used_sources.push_back(j);
					break;
				}
			}

			Login_Player_Index(i, InputSource(InputSourceDevice::Joystick, free_joystick), i);
		}

		manager->Go(UIAction::Push(new UIScene_GameSetupMenu(), new UISlideInTransition()));
		return;
	}

	// Finished selecting profile.
	if (m_select_profile_index != -1 && manager->Get_Top_Scene_Index() == scene_index && m_selected_profile_index >= 0)
	{
		Login_Player_Index(m_select_profile_index, m_select_profile_source, m_selected_profile_index);
		m_select_profile_index = -1;
	}

	// Blink timer.
	m_blink_timer += time.Get_Frame_Time();
	if (m_blink_timer > BLINK_INTERVAL)
	{
		m_blink_timer = 0.0f;
		m_blink = !m_blink;
	}
	
	// Keep refreshing screens because the user may change names etc.
	Refresh_Screen();

	// Login new players?
	for (int i = 0; i < MAX_INPUT_SOURCES; i++)
	{
		Input* input = Input::Get(i);
	
		InputBindings::Type keyboard_select = input->Output_To_Active_Input(OutputBindings::GUI_Input_Select, InputBindingType::Keyboard);
		InputBindings::Type joystick_select = input->Output_To_Active_Input(OutputBindings::GUI_Input_Select, InputBindingType::Joystick);

		InputSource source(InputSourceDevice::NONE, 0);

		if (input->Get_Keyboard_State()->Was_Key_Pressed(keyboard_select))
		{
			source = InputSource(InputSourceDevice::Keyboard, i);
		}
		else if (input->Get_Joystick_State()->Was_Key_Pressed(joystick_select) ||
				 input->Get_Joystick_State()->Was_Action_Pressed(OutputBindings::GUI_Input_Select))
		{
			source = InputSource(InputSourceDevice::Joystick, i);
		}

		if (input->Get_Mouse_State()->Was_Button_Clicked(InputBindings::Mouse_Left))
		{
			Vector2 pos = input->Get_Mouse_State()->Get_Position();

			UIPanel* start_panel_element = Find_Element<UIPanel*>("player_1_panel");
			UIPanel* end_panel_element = Find_Element<UIPanel*>("player_4_panel");
			UIPanel* label_element = Find_Element<UIPanel*>("player_1_start_label");

			Rect2D selection_rect = Rect2D(
				start_panel_element->Get_Screen_Box().X, 
				0.0f, 
				end_panel_element->Get_Screen_Box().X - start_panel_element->Get_Screen_Box().X, 
				label_element->Get_Screen_Box().Y + label_element->Get_Screen_Box().Height		
			);

			if (selection_rect.Intersects(pos))
			{
				source = InputSource(InputSourceDevice::Keyboard, i);
			}
		}

		if (source.Device != InputSourceDevice::NONE)
		{
			int index = Input_Source_To_Player_Index(source);
			if (index != -1)
			{
				Logout_Player_Index(index);		
				manager->Play_UI_Sound(UISoundType::Close_Popup);
			}
			else
			{
				int new_index = Get_Free_Player_Index();
				if (new_index != -1)
				{	
					m_select_profile_source = source;
					m_select_profile_index  = new_index;
					
					// Temporary login/logout so we can use new input source to select profile.
					Login_Player_Index(m_select_profile_index, m_select_profile_source, -1);

					manager->Go(UIAction::Push(new UIScene_SelectProfile(), new UIFadeInTransition()));
					manager->Play_UI_Sound(UISoundType::Popup);
					return;
				}
				else
				{
					manager->Play_UI_Sound(UISoundType::Back);
				}
			}			
		}
	}

	// Enable/Disable button.
	UIButton* button = Find_Element<UIButton*>("continue_button");
	if (button != NULL)
	{
		button->Set_Enabled(Any_Player_Logged_In());
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_PlayerLogin::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);

	// Draw characters.
	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		std::string panel_name = StringHelper::Format("player_%i_panel", i + 1);
		UIPanel* panel_element = Find_Element<UIPanel*>(panel_name.c_str());

		if (m_login_indexes[i] >= 0)
		{
			OnlineUser* user = OnlinePlatform::Get()->Get_Local_User_By_Index(m_login_indexes[i]);			
			int idx = user->Get_Profile_Index();
			if (idx >= 0)
			{			
				Profile* profile = ProfileManager::Get()->Get_Profile(idx);
				m_char_renderer->Draw(time, manager, profile, panel_element->Get_Screen_Box());
			}
		}
	}
}

void UIScene_PlayerLogin::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Dialog_Close:
		{
			UIScene_SelectProfile* profile_select = dynamic_cast<UIScene_SelectProfile*>(e.Scene);
			if (profile_select != NULL)
			{
				Logout_Player_Index(m_select_profile_index);

				m_selected_profile_index = profile_select->Get_Selected_Index();
				DBG_LOG("Selected profile %i", m_selected_profile_index);
			}
			break;
		}
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "continue_button")
			{
				if (m_accept_invite == true || manager->Can_Accept_Invite())
				{
					m_accept_invite = false;

					manager->Go(UIAction::Push(new UIScene_FindGame(true), new UISlideInTransition()));
				}
				else
				{
					manager->Go(UIAction::Push(new UIScene_GameSetupMenu(), new UISlideInTransition()));
				}
			}
			else if (e.Source->Get_Name() == "back_button")
			{
				manager->Go(UIAction::Pop(new UISlideInTransition()));
			}
		}
		break;
	}
}
