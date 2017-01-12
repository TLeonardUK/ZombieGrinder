// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Inventory.h"
#include "Game/UI/Scenes/UIScene_Equipment.h"
#include "Game/UI/Scenes/UIScene_Skills.h"
#include "Game/UI/Scenes/UIScene_Shop.h"
#include "Game/UI/Scenes/UIScene_ConfirmDialog.h"
#include "Game/UI/Scenes/UIScene_SelectProfile.h"
#include "Game/UI/Scenes/UIScene_ControllerTextInputDialog.h"

#include "Game/UI/Scenes/Helper/UICharacterPreviewRenderer.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UISlideInTopLevelTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UIFadeTopLevelTransition.h"
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
#include "Engine/UI/Elements/UIButton.h"
#include "Engine/UI/Elements/UITextBox.h"
#include "Engine/UI/Elements/UIComboBox.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Generic/Math/Random.h"

#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineUser.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Engine/Localise/Locale.h"
#include "Engine/Engine/GameEngine.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

#include "Game/Profile/ProfileManager.h"

UIScene_Inventory::UIScene_Inventory(Profile* profile, bool creating, int profile_index)
	: m_profile(profile)
	, m_creating(creating)
	, m_profile_index(profile_index)
	, m_showing_name_input_dialog(false)
{
	DBG_ASSERT(profile != NULL);
	DBG_LOG("Modifying inventory for: %s", profile->Name.c_str());

	if (creating)
		Set_Layout("new_inventory");	
	else
		Set_Layout("inventory");	

	if (creating == true)
	{
		UILabel* title = Find_Element<UILabel*>("title_label");
		title->Set_Value(S("#menu_inventory_create_title"));

		UIButton* button = Find_Element<UIButton*>("back_button");
		button->Set_Value(S("#menu_select_character_create"));
	}

	m_char_renderer = new UICharacterPreviewRenderer();
}

UIScene_Inventory::~UIScene_Inventory()
{	
	Game::Get()->Queue_Save();
	SAFE_DELETE(m_char_renderer);
}

const char* UIScene_Inventory::Get_Name()
{
	return "UIScene_Inventory";
}

bool UIScene_Inventory::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_Inventory::Should_Display_Cursor()
{
	return true;
}

bool UIScene_Inventory::Is_Focusable()
{
	return true;
}

void UIScene_Inventory::Enter(UIManager* manager)
{
	ProfileManager* profile_manager = ProfileManager::Get();

	UITextBox* text_box = Find_Element<UITextBox*>("name_box");
	text_box->Set_Value(m_profile->Name);
	text_box->Set_Max_Length(Profile::max_name_length);

	UILabel* level_label = Find_Element<UILabel*>("level_label");
	UILabel* skill_label = Find_Element<UILabel*>("skill_label");
	UILabel* gold_label = Find_Element<UILabel*>("gold_label");
	UIProgressBar* level_progress = Find_Element<UIProgressBar*>("level_progress");
	
	UIComboBox* voice_box = Find_Element<UIComboBox*>("voice_box");
	voice_box->Clear_Items();
	voice_box->Add_Item("#menu_inventory_voice_type_female");
	voice_box->Add_Item("#menu_inventory_voice_type_male");

	voice_box->Set_Selected_Item_Index(m_profile->Is_Male ? 1 : 0);

	int xp_for_next_level = profile_manager->Get_Level_XP(m_profile->Level + 1);
	int xp = m_profile->Level_XP;
	float xp_progress = m_profile->Get_Level_Progress();

	level_label->Set_Value(SF("#menu_select_character_level", m_profile->Level));
	if (xp_for_next_level == 0)
	{
		level_progress->Set_Progress(1.0f);
		level_progress->Set_Value(S("#menu_inventory_max_level"));
	}
	else
	{
		level_progress->Set_Progress(xp_progress);
		level_progress->Set_Value(SF("#menu_inventory_xp",
			StringHelper::Format_Number((float)xp).c_str(),
			StringHelper::Format_Number((float)xp_for_next_level).c_str()
			));
	}

	skill_label->Set_Value(SF("#menu_inventory_skill_points", StringHelper::Format_Number((float)m_profile->Skill_Points).c_str()));

	if (m_profile->Coins >= m_profile->Wallet_Size)
	{
		gold_label->Set_Value(SF("#menu_inventory_gold_full", StringHelper::Format_Number((float)m_profile->Coins).c_str()));
	}
	else
	{
		gold_label->Set_Value(SF("#menu_inventory_gold", StringHelper::Format_Number((float)m_profile->Coins).c_str()));
	}

	if (m_creating == true)
	{
		text_box->Focus();
		GameEngine::Get()->Get_UIManager()->Focus(text_box);
	}
}	

void UIScene_Inventory::Update_Profile(bool final_update)
{
	UITextBox* name_box = Find_Element<UITextBox*>("name_box");

	// Update profile data.
	m_profile->Name = name_box->Get_Value();

	if (final_update == true && m_profile->Name == "")
	{
		m_profile->Name = S("#menu_select_character_untitled_name");
		name_box->Set_Value(m_profile->Name);
	}
}

void UIScene_Inventory::Exit(UIManager* manager)
{
}	

void UIScene_Inventory::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	Update_Profile(false);

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Inventory::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);

	// Draw character.
	UIPanel* info_panel = Find_Element<UIPanel*>("description_label");
	if (m_profile != NULL)
	{
		m_char_renderer->Draw(time, manager, m_profile, info_panel->Get_Screen_Box());
	}
}

void UIScene_Inventory::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Dialog_Close:
		{
			if (m_showing_name_input_dialog)
			{
				UIScene_ControllerTextInputDialog* dialog = dynamic_cast<UIScene_ControllerTextInputDialog*>(e.Scene);
				UITextBox* name_box = Find_Element<UITextBox*>("name_box");
				std::string result = dialog->Get_Input();
				if (result == "")
				{
					result = S("#menu_select_character_untitled_name");
				}
				m_profile->Name = result;
				name_box->Set_Value(result);
				m_showing_name_input_dialog = false;
			}
			else
			{
				if (dynamic_cast<UIScene_ConfirmDialog*>(e.Scene)->Get_Selected_Index() == 1)
				{
					manager->Go(UIAction::Pop(new UISlideInTopLevelTransition()));
				}
			}
		}
		break;
	case UIEventType::Element_Focus:
		{
		}
		break;
	case UIEventType::TextBox_JoystickSelect:
		{
			// Select text box using controller, show enter-text screen.
			UITextBox* name_box = Find_Element<UITextBox*>("name_box");
			m_showing_name_input_dialog = true;
			manager->Go(UIAction::Push(new UIScene_ControllerTextInputDialog(name_box->Get_Value(), S("#menu_select_character_create"), Profile::max_name_length, e.Source_Joystick), new UIFadeInTransition()));
		}
		break;
	case UIEventType::ComboBox_SelectedIndexChanged:
		{
			if (e.Source->Get_Name() == "voice_box")
			{
				UIComboBox* voice_box = Find_Element<UIComboBox*>("voice_box");
				m_profile->Is_Male = !!voice_box->Get_Selected_Item_Index();

				if (m_profile->Is_Male)
				{
					SoundHandle* handle = ResourceFactory::Get()->Get_Sound(StringHelper::Format("sfx_objects_avatars_hurt_%i", Random::Static_Next(1, 4)).c_str());
					AudioRenderer::Get()->Play_Tracked(handle, false);
				}
				else
				{
					SoundHandle* handle = ResourceFactory::Get()->Get_Sound(StringHelper::Format("sfx_objects_avatars_female_hurt_%i", Random::Static_Next(1, 4)).c_str());
					AudioRenderer::Get()->Play_Tracked(handle, false);
				}
			}
		}
		break;
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "inventory_button")
			{	
				manager->Go(UIAction::Push(new UIScene_Equipment(m_profile), new UISlideInTopLevelTransition()));
			}
			else if (e.Source->Get_Name() == "skills_button")
			{	
				manager->Go(UIAction::Push(new UIScene_Skills(m_profile), new UISlideInTopLevelTransition()));
			}
			else if (e.Source->Get_Name() == "shop_button")
			{					
				manager->Go(UIAction::Push(new UIScene_Shop(m_profile), new UISlideInTopLevelTransition()));
			}
			else if (e.Source->Get_Name() == "back_button")
			{	
				Update_Profile(true);
	
				bool exit = true;

				if (m_creating == true)
				{
					UITextBox* name_box = Find_Element<UITextBox*>("name_box");
					if (name_box->Get_Value() == S("#menu_select_character_untitled_name"))
					{
						manager->Go(UIAction::Push(new UIScene_ConfirmDialog(S("#menu_select_character_confirm_untitled"), S("#menu_no"), S("#menu_yes")), new UIFadeInTransition()));
						manager->Play_UI_Sound(UISoundType::Popup);
						exit = false;
					}
				}

				if (exit == true)
				{
					manager->Go(UIAction::Pop(new UISlideInTopLevelTransition()));
				}
			}
			else if (e.Source->Get_Name() == "cancel_button")
			{	
				m_profile = NULL;
				ProfileManager::Get()->Delete_Profile(m_profile_index);

				std::vector<OnlineUser*> local_users = OnlinePlatform::Get()->Get_Local_Users();

				// Adjust indexes of other user profiles.
				for (std::vector<OnlineUser*>::iterator iter = local_users.begin(); iter != local_users.end(); iter++)
				{
					OnlineUser* user = *iter;
					if (user->Get_Profile_Index() >= m_profile_index)
					{
						user->Set_Profile_Index(user->Get_Profile_Index() - 1);
					}
				}

				Game::Get()->Queue_Save();
				
				UIScene_SelectProfile* profile_select = manager->Get_Scene_By_Type<UIScene_SelectProfile*>();
				profile_select->Cancel();

				manager->Go(UIAction::Pop(new UISlideInTopLevelTransition()));
			}
		}
		break;
	}
}
