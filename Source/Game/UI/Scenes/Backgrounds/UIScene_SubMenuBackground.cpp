// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Backgrounds/UIScene_SubMenuBackground.h"
#include "Game/UI/Scenes/Backgrounds/UIScene_GameBackground.h"
#include "Game/UI/Scenes/UIScene_Vote.h"
#include "Game/UI/Scenes/UIScene_TeamChange.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Generic/Math/Math.h"
#include "Generic/Math/Random.h"

#include "Engine/Localise/Locale.h"

#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetClient.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"

UIScene_SubMenuBackground::UIScene_SubMenuBackground()
	: m_title_fade_alpha(1.0f)
	, m_show_title(true)
{
	Set_Layout("submenu_background");	

	m_title_renderer.Reset();
}

const char* UIScene_SubMenuBackground::Get_Name()
{
	return "UIScene_SubMenuBackground";
}

bool UIScene_SubMenuBackground::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_SubMenuBackground::Should_Tick_When_Not_Top()
{
	return true;
}

void UIScene_SubMenuBackground::Show_Title(bool show)
{
	m_show_title = show;
}

void UIScene_SubMenuBackground::Refresh(UIManager* manager)
{
	// Refresh base stuff.
	UIScene::Refresh(manager);

	m_title_renderer.Reset();
}

void UIScene_SubMenuBackground::Enter(UIManager* manager)
{
	NetManager*			net_manager	 = NetManager::Get();
	GameNetClient*		net_client	 = static_cast<GameNetClient*>(net_manager->Client());
	GameNetServerState  server_state = net_client->Get_Server_State();

	// Update the server-name label.
	UILabel* server_name_label = Find_Element<UILabel*>("server_name");
	server_name_label->Set_Value(SF("#sub_menu_server_name", server_state.Name.c_str(), server_state.Host_Name.c_str()));

	// Reset the pretty logo renderer :3
	m_title_renderer.Reset();
}	

void UIScene_SubMenuBackground::Exit(UIManager* manager)
{
}	

void UIScene_SubMenuBackground::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
//	UIScene::Tick(time, manager, scene_index);
}

void UIScene_SubMenuBackground::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Fade out title + copyright labels if top screen is not one that will appreciate it :3.
	float target_title_fade = 1.0f;

	// Call-vote & Change-Team keep logo.
	if (!m_show_title)
	{
		target_title_fade = 0.0f;
	}

	m_title_fade_alpha = Math::Lerp(m_title_fade_alpha, target_title_fade, 0.2f);

	UILabel* copyright_label = Find_Element<UILabel*>("copyright");
	UILabel* server_name_label = Find_Element<UILabel*>("server_name");

	Color original_color = copyright_label->Get_Color();
	original_color.A = (unsigned char)(255.0f * m_title_fade_alpha);
	copyright_label->Set_Color(original_color);

	original_color = server_name_label->Get_Color();
	original_color.A = (unsigned char)(255.0f * m_title_fade_alpha);
	server_name_label->Set_Color(original_color);
	
	// Draw background.
	//manager->Retrieve_Persistent_Scene("UIScene_GameBackground")->Draw(time, manager, scene_index);

	// Draw game scene. This is pretty hacky, but neccessary to get the effect we want :S.
	UIScene* game = manager->Get_Scene_By_Name("UIScene_Game");
	DBG_ASSERT(game != NULL);
	game->Draw(time, manager, scene_index);

	// Render the background elements.
	UIScene::Draw(time, manager, scene_index);

	// Render the title image.
	UIPanel* panel	= Find_Element<UIPanel*>("title_image");
	if (panel != NULL)
	{
		Rect2D screen_box	= panel->Get_Screen_Box();

		float logo_x	= screen_box.X + (screen_box.Width * 0.5f);
		float logo_y	= screen_box.Y + (screen_box.Height * 0.5f);

		// We always tick in the draw call
		// as we are a background and sometimes will not get ticked depending on the current
		// top-scene's settings.
		m_title_renderer.Tick(time);
		m_title_renderer.Draw(time, Vector2(logo_x, logo_y), Vector2(0, 0), false, 0.0, 0.60f, m_title_fade_alpha);
	}
}



