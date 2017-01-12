// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_MainMenu.h"
#include "Game/UI/Scenes/UIScene_ControllerTextInputDialog.h"
#include "Game/UI/Scenes/UIScene_RestrictedModeWarning.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"
#include "Game/UI/Scenes/UIScene_ModOptions.h"
#include "Game/UI/Scenes/UIScene_Video.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIButton.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Engine/EngineOptions.h"

#include "Engine/Localise/Locale.h"
#include "Game/Runner/GameOptions.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Game/UI/Scenes/UIScene_Game.h"
#include "Game/UI/Scenes/UIScene_PlayerLogin.h"
#include "Game/UI/Scenes/UIScene_Options.h"
#include "Game/UI/Scenes/UIScene_Credits.h"
#include "Game/UI/Scenes/UIScene_Premium.h"

#include "Engine/UI/Transitions/UIFadeTransition.h"
#include "Engine/UI/Transitions/UISlideInTopLevelTransition.h"

#include "Engine/Video/VideoFactory.h"
#include "Engine/Video/VideoRenderer.h"

#include "Game/Moderation/ReportManager.h"

#include "Engine/Engine/GameEngine.h"
#include "Game/Runner/Game.h"

#define OPT_VIDEO_TEST

UIScene_MainMenu::UIScene_MainMenu()
	: m_last_purchase_state(-1)
	, m_restricted_warning_pending(false)
{
	Check_Layout();
	AudioRenderer::Get()->Push_BGM("music_bommirific");

	if (Game::Get()->In_Restricted_Mode())
	{
		m_restricted_warning_pending = true;
	}
}

UIScene_MainMenu::~UIScene_MainMenu()
{
	AudioRenderer::Get()->Pop_BGM();
}

const char* UIScene_MainMenu::Get_Name()
{
	return "UIScene_MainMenu";
}

bool UIScene_MainMenu::Should_Render_Lower_Scenes()
{
	return false;
}

UIScene* UIScene_MainMenu::Get_Background(UIManager* manager)
{
	return manager->Retrieve_Persistent_Scene("UIScene_MenuBackground");
}	

bool UIScene_MainMenu::Is_Focusable()
{
	return true;
}

bool UIScene_MainMenu::Should_Display_Cursor()
{
	return true;
}

void UIScene_MainMenu::Check_Layout()
{
#ifdef OPT_PREMIUM_ACCOUNTS
	int new_state = OnlinePlatform::Get()->Check_Purchased(*GameOptions::premium_account_appid);;
	if (new_state != m_last_purchase_state)
	{
		if (new_state)
		{
			DBG_LOG("Using premium layout for main menu.");
			Set_Layout("main_menu_premium");
		}
		else
		{
			DBG_LOG("Using default layout for main menu.");
			Set_Layout("main_menu");
		}

		GameEngine::Get()->Get_UIManager()->Focus(Find_Element<UIButton*>("start_button"));
	}

	m_last_purchase_state = new_state;
#else
	Set_Layout("main_menu_premium");
#endif
}

void UIScene_MainMenu::Enter(UIManager* manager)
{
	Check_Layout();

	if (*EngineOptions::render_legacy_was_forced && !(*EngineOptions::render_legacy_was_informed))
	{
		*EngineOptions::render_legacy_was_informed = true;
		manager->Go(UIAction::Push(new UIScene_Dialog(S("#legacy_rendering_notice"), S("#menu_continue"), true, true), new UIFadeInTransition()));
	}
}	

void UIScene_MainMenu::Exit(UIManager* manager)
{
}	

void UIScene_MainMenu::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
#if 0//def OPT_VIDEO_TEST

	static bool bVideoTest = false;

	// Video test.
	VideoHandle* handle = ResourceFactory::Get()->Get_Video("Data/Videos/c1_s1.ogv");
	if (handle)
	{
		manager->Go(UIAction::Push(new UIScene_Video(handle), new UIFadeInTransition()));
		bVideoTest = true;
	}

#endif

	// Exit on escape.
	if (Input::Get()->Was_Pressed_By_Any(OutputBindings::Escape))
	{	
		DBG_LOG("Game stopping due to escape key.");
		GameEngine::Get()->Stop();
	}

	// Auto load map.
	if (*GameOptions::editor == true || 
		*GameOptions::map != "" ||
		*EngineOptions::connect != 0)
	{
		manager->Go(UIAction::Push(new UIScene_PlayerLogin(), new UISlideInTransition()));
		return;
	}

	// Show ban message.
	ReportManager* report_manager = ReportManager::Get();
	if (report_manager->Has_Notification() && manager->Get_Top_Scene_Index() == scene_index)
	{
		if (*GameOptions::ban_message_shown == false)
		{
			//ReportManager::Get()->Get_Notification();
			CheatReportQueueNotification notice = report_manager->Get_Notifications()[0];

			std::string cheat_types = "";
			int val = (int)notice.Category;

			if ((val & (int)CheatCategory::Exploit) != 0)
			{
				cheat_types += S("#ban_notice_exploit");
			}
			if ((val & (int)CheatCategory::Cheat) != 0)
			{
				if (cheat_types != "")
				{
					cheat_types += S("#ban_notice_type_seperator");
				}
				cheat_types += S("#ban_notice_cheat");
			}
			if ((val & (int)CheatCategory::Abuse) != 0)
			{
				if (cheat_types != "")
				{
					cheat_types += S("#ban_notice_type_seperator");
				}
				cheat_types += S("#ban_notice_abuse");
			}
			if ((val & (int)CheatCategory::Grief) != 0)
			{
				if (cheat_types != "")
				{
					cheat_types += S("#ban_notice_type_seperator");
				}
				cheat_types += S("#ban_notice_grief");
			}

			std::string message = "";
			if (notice.End_Time == "")
			{
				message = S("#ban_notice_message_indefinite");
				message = StringHelper::Format(message.c_str(),
					notice.Report_Time.c_str(),
					cheat_types.c_str()
					);
			}
			else
			{
				message = S("#ban_notice_message");
				message = StringHelper::Format(message.c_str(),
					notice.End_Time.c_str(),
					notice.Report_Time.c_str(),
					cheat_types.c_str()
					);
			}

			manager->Play_UI_Sound(UISoundType::Enter_SubMenu);
			manager->Go(UIAction::Push(new UIScene_Dialog(message, S("#menu_continue"), true, false, "game_ban_warning"), new UIFadeInTransition()));

			*GameOptions::ban_message_shown = true;
			return;
		}
	}

	// Restricted mode?
	if (m_restricted_warning_pending && manager->Get_Top_Scene_Index() == scene_index)
	{
		manager->Go(UIAction::Push(new UIScene_RestrictedModeWarning(), new UIFadeInTransition()));
		m_restricted_warning_pending  = false;
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_MainMenu::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	/*
	const char* test_text = "This xckzug;ozidfugosidfug kljxcnvbo;isdffugoisdufgoisdvkljbnsd op;figu sdofjghsdoifguysodifgusod ifugosdxjngop;sdifugosdi ug osdifugosdiugosdifug;ozdxinbsaopieru6to usdfhblkjsdhfpotiw er;opitghjsd;klfjghs;lodfikhjgs;lodikfjvb;ps' do iutr;opiksd hfglksdfjhg'paqoimt;mpv sidufg;lk ksiehjufg;pqoeiurt;ov yisdug;posdiufp[';w9gosua;op3 ivbg u56t;sldeofgi hjks['pdoijn ugt;pb iosdeuty;opszidufg;'psoaueinw ;4ol5it busdlgm,is a adsklhfgj lsadfhgjlskjd hfglsjkdhfgkljsdhfgkjshdfkgjhsdfkj ghsdkfjgh skldjfhgk sjdhfgk jsdhfgkl jsdhkg jhsdkg jhsdkjghsdklfjhgklsdjfhgksdjhfgkjsdhfgkjsdhfkg jhsdfkgjh sdkfjghsdkjfhgksjdhfgkjsd hfgkjsdhfgkjsdfhkgjshdfkgjhsdkljg hsdkfjghsdkjhgksdjhg kljsdhgk ljsdhfgkjshdfkl jnskdjfhgksdjfhygks djuhfgklsjd fhgksjdfhgkl;sdjfoptiu sdkjbgnsd,fjvhblzk eurto;isdhjfgkjsdxhkgljsjd;lopfig sdifhjgkjdshvblkasjdrpot uiosdifhug lkxjcfhbvlikseuipot iseofjhgskldjfhblxckvjubposaeir toihdskfgjbhnsd ;lofbui sdepiutljdhfblkkxc jvpgoius eprituldjkfhglsdfugp soidufbosidfhgloisdhjfgpois udfogihsdfogihs dkljfbvnxpcoviugosdkhfgkl jsdnfgsd ifguxcopvibhsd kjfghse dpfogi ucovkbnsdo ifgus epdiofgu spdfgiuckxvnbosidhfgps dioufgxckvnbsdpiofhygosdi fhgsodkjfgnxcl;vkhgsodifhgsdpo iugjsdkljnbspdo ifuhygsd pifgusdoifhgsdfg";
	Rect2D test_box = Rect2D(10, 10, 500, 2000);

	PrimitiveRenderer pr;
	pr.Draw_Solid_Quad(test_box, Color::Red);

	FontRenderer fr(manager->Get_Font());
	std::string output = fr.Word_Wrap(test_text, test_box, 16.0f, Vector2(1.0f, 1.0f));
	fr.Draw_String(output.c_str(), test_box, 16.0f);
	*/

	UIScene::Draw(time, manager, scene_index);
}

void UIScene_MainMenu::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "start_button")
			{
				manager->Go(UIAction::Push(new UIScene_PlayerLogin(), new UISlideInTransition()));
			}
			else if (e.Source->Get_Name() == "options_button")
			{
				manager->Go(UIAction::Push(new UIScene_Options(), new UISlideInTransition()));
			}
			else if (e.Source->Get_Name() == "credits_button")
			{
				manager->Go(UIAction::Push(new UIScene_Credits(), new UIFadeTransition()));
			}
			else if (e.Source->Get_Name() == "mod_button")
			{
				if (!OnlinePlatform::Get()->Is_Online())
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#online_platform_feature_not_available_offline")), new UIFadeInTransition()));
				}
				else
				{
					manager->Go(UIAction::Push(new UIScene_ModOptions(), new UISlideInTransition()));
				}
			}
#ifdef OPT_PREMIUM_ACCOUNTS
			else if (e.Source->Get_Name() == "premium_button")
			{
				manager->Go(UIAction::Push(new UIScene_Premium(), new UISlideInTransition()));
			}
#endif
			else if (e.Source->Get_Name() == "quit_button")
			{
				GameEngine::Get()->Stop();
			}
		}
		break;
	}
}



