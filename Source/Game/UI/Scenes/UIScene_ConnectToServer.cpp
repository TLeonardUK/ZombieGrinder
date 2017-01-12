// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_ConnectToServer.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
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

#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetClient.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"
#include "Game/UI/Scenes/UIScene_MapLoading.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_ConnectToServer::UIScene_ConnectToServer(OnlineMatching_Server server)
	: m_server(server)
	, m_connection_stated(false)
{
	Set_Layout("connect_to_server");	
}

bool UIScene_ConnectToServer::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_ConnectToServer::Get_Name()
{
	return "UIScene_ConnectToServer";
}

bool UIScene_ConnectToServer::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_ConnectToServer::Should_Display_Cursor()
{
	return true;
}

bool UIScene_ConnectToServer::Is_Focusable()
{
	return true;
}

void UIScene_ConnectToServer::Enter(UIManager* manager)
{
	if (!m_connection_stated)
	{
		m_connection_stated = true;
		NetManager::Get()->Client()->Connect_To_Server(m_server);
	}
}	

void UIScene_ConnectToServer::Exit(UIManager* manager)
{
}	

void UIScene_ConnectToServer::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	NetClient* client = NetManager::Get()->Client();
	switch (client->Get_State())
	{
		case NetClient_State::Connecting:
			{
				// Waiting ....
				break;
			}
		case NetClient_State::Connected:
			{
				manager->Go(UIAction::Replace(new UIScene_MapLoading(), new UISlideInTransition()));				
			}
			break;
		case NetClient_State::Error:
			{
				NetClient_Error::Type error = client->Get_Error();
				Find_Element<UILabel*>("label")->Set_Value(client->Get_Error_Message(error));
				Find_Element<UILabel*>("back_button")->Set_Value("#menu_close");
			}
			break;
		default:
			{
				Find_Element<UILabel*>("label")->Set_Value("#net_error_internal_error");
				Find_Element<UILabel*>("back_button")->Set_Value("#menu_close");
			}
			break;
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_ConnectToServer::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_ConnectToServer::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "back_button")
			{
				NetManager::Get()->Leave_Network_Game();

				if (manager->Get_Scene_By_Name("UIScene_ServerBrowser") != NULL)
					manager->Go(UIAction::Pop(new UIFadeOutTransition(), "UIScene_ServerBrowser"));
				else if (manager->Get_Scene_By_Name("UIScene_GameSetupMenu") != NULL)
					manager->Go(UIAction::Pop(new UIFadeOutTransition(), "UIScene_GameSetupMenu"));
				else
					manager->Go(UIAction::Pop(new UIFadeOutTransition(), "UIScene_PlayerLogin"));
			}
		}
		break;
	}
}
