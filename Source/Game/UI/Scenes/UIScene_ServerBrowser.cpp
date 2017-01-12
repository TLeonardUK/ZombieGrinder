// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_ServerBrowser.h"
#include "Game/UI/Scenes/UIScene_ConnectToServer.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UIListView.h"
#include "Engine/UI/Elements/UIComboBox.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/GeoIP/GeoIPManager.h"

#include "Engine/Online/OnlineMatching.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_ServerBrowser::UIScene_ServerBrowser()
{
	Set_Layout("server_browser");	
	
	UIListView* listview = Find_Element<UIListView*>("server_listview");
	listview->Clear_Columns();
	listview->Add_Column("", 0.078f, false, true);
	listview->Add_Column("", 0.063f, false, true);
	listview->Add_Column("", 0.063f, false, true);
	listview->Add_Column(S("#menu_server_browser_name_column"), 0.35f);
	listview->Add_Column(S("#menu_server_browser_map_column"), 0.249f);
	listview->Add_Column(S("#menu_server_browser_players_column"), 0.13f);
	listview->Add_Column(S("#menu_server_browser_ping_column"), 0.075f);

	UIComboBox* sourcebox = Find_Element<UIComboBox*>("source_box");
	sourcebox->Add_Item(S("#menu_server_browser_source_internet"));
	sourcebox->Add_Item(S("#menu_server_browser_source_lan"));
	sourcebox->Add_Item(S("#menu_server_browser_source_friends"));
	sourcebox->Add_Item(S("#menu_server_browser_source_favorites"));
	sourcebox->Add_Item(S("#menu_server_browser_source_history"));
}

const char* UIScene_ServerBrowser::Get_Name()
{
	return "UIScene_ServerBrowser";
}

bool UIScene_ServerBrowser::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_ServerBrowser::Should_Display_Cursor()
{
	return true;
}

bool UIScene_ServerBrowser::Is_Focusable()
{
	return true;
}

OnlineMatching_ServerSource::Type UIScene_ServerBrowser::Get_Server_Source()
{
	return (OnlineMatching_ServerSource::Type)Find_Element<UIComboBox*>("source_box")->Get_Selected_Item_Index();
}

void UIScene_ServerBrowser::Enter(UIManager* manager)
{
	m_last_server_source = Get_Server_Source();
	OnlineMatching::Get()->Refresh_Server_List(Get_Server_Source());
}	

void UIScene_ServerBrowser::Exit(UIManager* manager)
{
	OnlineMatching::Get()->Cancel_Server_List_Refresh(m_last_server_source);
}	

void UIScene_ServerBrowser::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Time to refresh the list?
	if (OnlineMatching::Get()->Has_Server_List_Updated(m_last_server_source))
	{
		UIListView* listview = Find_Element<UIListView*>("server_listview");
		listview->Clear_Items();

		m_servers = OnlineMatching::Get()->Get_Server_List(m_last_server_source);
		for (std::vector<OnlineMatching_Server>::iterator iter = m_servers.begin(); iter != m_servers.end(); iter++)
		{
			OnlineMatching_Server& server = *iter;

			std::vector<std::string> values;

			GeoIPResult result = GeoIPManager::Get()->Lookup_IP(server.Address.Get_IP());

			values.push_back(StringHelper::Lowercase(StringHelper::Format("[img=flag_%s,1.0,1.0,0,0]", result.CountryShortName).c_str()));
		
			if (server.Secure)
			{
				values.push_back("[img=game_hud_emblem_developer,1.0,1.0,0,0]");
			}
			else
			{
				values.push_back("");
			}

			if (server.Private)
			{
				values.push_back("[img=game_hud_lock,1,1,0,0]");
			}
			else
			{
				values.push_back("");
			}

			values.push_back(server.Name.c_str());
			values.push_back(server.Map.c_str());
			values.push_back(StringHelper::Format("%i/%i", server.PlayerCount, server.MaxPlayerCount));
			values.push_back(StringHelper::Format("%i", server.Ping));

			listview->Add_Item(values);
		}
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_ServerBrowser::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_ServerBrowser::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::ListView_Item_Click:
		{
			if (e.Source->Get_Name() == "server_listview")
			{
				UIListView* listview = static_cast<UIListView*>(e.Source);
				OnlineMatching_Server& server = m_servers.at(listview->Get_Selected_Item_Index());

				manager->Go(UIAction::Push(new UIScene_ConnectToServer(server), new UIFadeInTransition()));
			}
		}
		break;
	case UIEventType::ComboBox_SelectedIndexChanged:
		{
			if (e.Source->Get_Name() == "source_box")
			{
				OnlineMatching::Get()->Cancel_Server_List_Refresh(m_last_server_source);
				m_last_server_source = Get_Server_Source();
				OnlineMatching::Get()->Refresh_Server_List(m_last_server_source);
			}
		}
		break;
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "refresh_button")
			{
				OnlineMatching::Get()->Refresh_Server_List(m_last_server_source);
			}
			else if (e.Source->Get_Name() == "back_button")
			{
				manager->Go(UIAction::Pop(new UISlideInTransition()));
			}
		}
		break;
	}
}
