// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_HostGame.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"
#include "Game/UI/Scenes/UIScene_Lobby.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeTopLevelTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UIListView.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Localise/Locale.h"

#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_HostGame::UIScene_HostGame(MapFileHandle* map, int players, bool is_private, bool is_secure, bool is_local, bool auto_continue, bool start_editor)
	: m_map(map)
	, m_players(players)
	, m_is_private(is_private)
	, m_is_secure(is_secure)
	, m_is_local(is_local)
	, m_auto_continue(auto_continue)
	, m_start_editor(start_editor)
{
	Set_Layout("host_game");	
}

bool UIScene_HostGame::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_HostGame::Get_Name()
{
	return "UIScene_HostGame";
}

bool UIScene_HostGame::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_HostGame::Should_Display_Cursor()
{
	return true;
}

bool UIScene_HostGame::Is_Focusable()
{
	return true;
}

void UIScene_HostGame::Enter(UIManager* manager)
{
	OnlineMatching_LobbySettings settings;
	settings.MapGUID		= m_map->Get()->Get_Header()->GUID;
	settings.MapWorkshopID	= m_map->Get()->Get_Header()->Workshop_ID;
	settings.MaxPlayers		= m_players;
	settings.Private		= m_is_private;
	settings.Secure			= m_is_secure;
	settings.Local			= m_is_local;
	settings.IsOffline		= !OnlinePlatform::Get()->Is_Online();

	OnlineMatching::Get()->Create_Lobby(settings);
}	

void UIScene_HostGame::Exit(UIManager* manager)
{
}	

void UIScene_HostGame::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	switch (OnlineMatching::Get()->Get_State())
	{
		case OnlineMatching_State::Creating_Lobby:
			break;
		case OnlineMatching_State::In_Lobby:
			{
				manager->Go(UIAction::Replace(new UIScene_Lobby(m_auto_continue, m_start_editor), new UISlideInTransition()));				
			}
			break;
		case OnlineMatching_State::Error:
			{
				OnlineMatching_Error::Type error = OnlineMatching::Get()->Get_Error();
				manager->Go(UIAction::Replace(new UIScene_Dialog(OnlineMatching::Get()->Get_Error_Message(error)), NULL));
			}
			break;
		default:
			{
				manager->Go(UIAction::Replace(new UIScene_Dialog("#matching_error_internal_error"), NULL));
			}
			break;
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_HostGame::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_HostGame::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "back_button")
			{
				OnlineMatching::Get()->Leave_Lobby();

				manager->Go(UIAction::Pop(new UIFadeOutTransition()));
			}
		}
		break;
	}
}
